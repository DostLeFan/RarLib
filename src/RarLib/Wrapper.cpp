#include "../../include/RarLib/Wrapper.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "../../include/RarLib/OS.hpp"

#if defined(WINDOWS)
	#include <windows.h>
	char const PATH_SEPARATOR = ';';
#elif defined(UNIX)
	#include <unistd.h>
	char const PATH_SEPARATOR = ':';
#else
	char const PATH_SEPARATOR = ':'; // Fallback.
#endif

namespace fs = std::filesystem;

Wrapper::Wrapper() {}

Wrapper::Wrapper(Wrapper const& src) : m_cachedRar(src.m_cachedRar), m_rarCached(src.m_rarCached) {}


Wrapper& Wrapper::operator=(Wrapper const& src)
{
	if(this != &src)
	{
		m_cachedRar = src.m_cachedRar;
		m_rarCached = src.m_rarCached;
	}
	
	return *this;
}


std::ostream& operator<<(std::ostream& os, Wrapper const& src)
{
	src.write(os);
	
	return os;
}


void Wrapper::write(std::ostream& os) const
{
	RarDetectionResult rar   = findRarExecutable();
	RarDetectionResult unrar = findUnrarExecutable();
	
	if(!rar.found && !unrar.found)
	{
		os << "Rar / WinRAR / unrar not found. Please install one of them.";
		
		return;
	}
	
	if(rar.found)
	{
		os << "rar / WinRAR found.\n"
		   << "  Type : " << (rar.type == RarType::RAR ? "rar" : "WinRAR") << "\n"
		   << "  Path : " << rar.path.string() << (unrar.found ? "\n" : "");
	}
	else
		os << "rar / WinRAR : not found." << (unrar.found ? "\n" : "");
	
	if(unrar.found)
	{
		os << "unrar found.\n"
		   << "  Type : unrar\n"
		   << "  Path : " << unrar.path.string();
	}
	else
		os << "unrar : not found.";
}


std::filesystem::path Wrapper::resolveArchivePath(std::string const& archiveName) const
{
	fs::path archivePath(archiveName);
	
	if(archivePath.is_absolute())
		return archivePath;
	else
		return fs::current_path() / archivePath;
}

bool Wrapper::executeCommandSafe(std::string const& command) const
{
	// Silences stdout and stderr — suitable for compression, extraction and test operations.
	#if !defined(WINDOWS) && !defined(UNIX)
		(void)command;
		
		return false;
	#elif defined(WINDOWS)
		std::ostringstream ossCmd;
		
		ossCmd << "cmd /C \"" << command << " >nul 2>nul\"";
		
		return std::system(ossCmd.str().c_str()) == 0;
	#else
		FILE* devnull = fopen("/dev/null", "w");
		
		if(!devnull)
		{
			devnull = std::fopen("/tmp/rar_silent.log", "w");
			
			if(!devnull)
				return std::system(command.c_str()) == 0;
		}
		
		fflush(stdout);
		fflush(stderr);
		
		int oldStdout = dup(fileno(stdout));
		int oldStderr = dup(fileno(stderr));
		
		dup2(fileno(devnull), fileno(stdout));
		dup2(fileno(devnull), fileno(stderr));
		
		int result = std::system(command.c_str());
		
		fflush(stdout);
		fflush(stderr);
		
		dup2(oldStdout, fileno(stdout));
		dup2(oldStderr, fileno(stderr));
		
		close(oldStdout);
		close(oldStderr);
		fclose(devnull);
		
		return result == 0;
	#endif
}

bool Wrapper::executeCommandWithOutput(std::string const& command) const
{
	// Preserves stdout — suitable for listArchive() where the caller needs to see the output.
	#if !defined(WINDOWS) && !defined(UNIX)
		(void)command;
		return false;
	#else
		return std::system(command.c_str()) == 0;
	#endif
}

bool Wrapper::fileExists(std::filesystem::path const& path) const
{
	#if defined(WINDOWS)
		return fs::exists(path) || fs::exists(path.string() + ".exe");
	#elif defined(UNIX)
		return fs::exists(path) && access(path.c_str(), X_OK) == 0;
	#else
		(void)path;
		
		return false;
	#endif
}

bool Wrapper::containsUnsafeChars(std::string const& str) const
{
	static std::string const unsafe = "\"';|&`$<>(){}\\!#~";
	
	return str.find_first_of(unsafe) != std::string::npos;
}

RarDetectionResult Wrapper::findRarExecutable() const
{
	if(m_rarCached)
		return m_cachedRar;
	
	m_rarCached = true;
	m_cachedRar.found = false;
	m_cachedRar.type = RarType::EMPTY;
	m_cachedRar.path = "";
	
	#if !defined(WINDOWS) && !defined(UNIX)
		return m_cachedRar;
	#elif defined(WINDOWS)
		HKEY hKey;
		char const* subkey = "SOFTWARE\\WinRAR";
		
		if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			char pathBuffer[MAX_PATH];
			DWORD bufferSize = sizeof(pathBuffer);
			
			if(RegQueryValueExA(hKey, "ExePath", nullptr, nullptr, (LPBYTE)pathBuffer, &bufferSize) == ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				
				m_cachedRar.found = true;
				m_cachedRar.type  = RarType::WINRAR;
				m_cachedRar.path  = fs::path(pathBuffer);
				
				return m_cachedRar;
			}
			
			RegCloseKey(hKey);
		}
		
		std::vector<std::string> defaultPaths = {
			"C:\\Program Files\\WinRAR\\Rar.exe",
			"C:\\Program Files (x86)\\WinRAR\\Rar.exe"
		};
		
		for(std::string const& p : defaultPaths)
		{
			if(fileExists(p))
			{
				m_cachedRar.found = true;
				m_cachedRar.type  = RarType::WINRAR;
				m_cachedRar.path  = fs::path(p);
				
				return m_cachedRar;
			}
		}
	#endif
	
	char const* envPath = std::getenv("PATH");
	
	if(envPath)
	{
		std::string pathEnv(envPath);
		size_t start = 0;
		size_t end   = 0;
		
		while((end = pathEnv.find(PATH_SEPARATOR, start)) != std::string::npos)
		{
			fs::path dir = pathEnv.substr(start, end - start);
			
			if(fileExists(dir / "rar"))
			{
				m_cachedRar.found = true;
				m_cachedRar.type  = RarType::RAR;
				m_cachedRar.path  = dir / "rar";
				
				return m_cachedRar;
			}
			
			start = end + 1;
		}
		
		fs::path lastDir = pathEnv.substr(start);
		
		if(fileExists(lastDir / "rar"))
		{
			m_cachedRar.found = true;
			m_cachedRar.type  = RarType::RAR;
			m_cachedRar.path  = lastDir / "rar";
			
			return m_cachedRar;
		}
	}
	
	return m_cachedRar;
}

RarDetectionResult Wrapper::findUnrarExecutable() const
{
	RarDetectionResult result;
	result.found = false;
	result.type = RarType::EMPTY;
	result.path = "";
	
	#if !defined(WINDOWS) && !defined(UNIX)
		return result;
	#elif defined(WINDOWS)
		// On Windows, WinRAR handles extraction natively — unrar.exe may still
		// be present alongside it in the WinRAR installation directory.
		RarDetectionResult const& winrar = findRarExecutable();
		
		if(winrar.found)
		{
			fs::path unrarCandidate = winrar.path.parent_path() / "UnRAR.exe";
			
			if(fileExists(unrarCandidate))
			{
				result.found = true;
				result.type  = RarType::UNRAR;
				result.path  = unrarCandidate;
				
				return result;
			}
		}
	#endif
	
	char const* envPath = std::getenv("PATH");
	
	if(envPath)
	{
		std::string pathEnv(envPath);
		size_t start = 0;
		size_t end   = 0;
		
		while((end = pathEnv.find(PATH_SEPARATOR, start)) != std::string::npos)
		{
			fs::path dir = pathEnv.substr(start, end - start);
			
			if(fileExists(dir / "unrar"))
			{
				result.found = true;
				result.type = RarType::UNRAR;
				result.path = dir / "unrar";
				
				return result;
			}
			
			start = end + 1;
		}
		
		fs::path lastDir = pathEnv.substr(start);
		
		if(fileExists(lastDir / "unrar"))
		{
			result.found = true;
			result.type  = RarType::UNRAR;
			result.path  = lastDir / "unrar";
			
			return result;
		}
	}
	
	return result;
}

RarDetectionResult Wrapper::findExtractionExecutable() const
{
	// Prefer unrar when available — it is the dedicated extraction tool.
	// Fall back to rar, which also supports extraction via the same flags.
	RarDetectionResult const& unrar = findUnrarExecutable();
	
	if(unrar.found)
		return unrar;
	
	return findRarExecutable();
}