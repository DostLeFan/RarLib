#include "../../include/RarLib/Rar.hpp"

#include <sstream>
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

Rar::Rar() {}

Rar::Rar(Rar const& src) : m_cachedRar(src.m_cachedRar), m_rarCached(src.m_rarCached) {}

Rar::~Rar() {}


Rar& Rar::operator=(Rar const& src)
{
	if(this != &src)
	{
		m_cachedRar = src.m_cachedRar;
		m_rarCached = src.m_rarCached;
	}
	
	return *this;
}


bool Rar::compressOneFile(std::string const& filePath, std::string const& archiveName) const
{
	if(containsUnsafeChars(filePath) || containsUnsafeChars(archiveName))
		return false;
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	fs::path inputPath = fs::absolute(filePath);
	
	std::ostringstream ossCmd;
	
	#if defined(WINDOWS)
		ossCmd << "\"" << rar.path.string() << "\" a -r \"" << archivePath.string() << "\" \"" << inputPath.string() << "\"";
	#else
		ossCmd << rar.path << " a -r \"" << archivePath.string() << "\" \"" << inputPath.string() << "\"";
	#endif
	
	return executeCommandSafe(ossCmd.str());
}

bool Rar::compressMultipleFiles(std::vector<std::string> const& files, std::string const& archiveName) const
{
	if(containsUnsafeChars(archiveName))
		return false;
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	
	std::ostringstream ossCmd;
	
	#if defined(WINDOWS)
		ossCmd << "\"" << rar.path.string() << "\" a -r \"" << archivePath.string() << "\"";
	#else
		ossCmd << rar.path << " a -r \"" << archivePath.string() << "\"";
	#endif
	
	for(std::string const& f : files)
	{
		if(containsUnsafeChars(f))
			return false;
		
		ossCmd << " \"" << fs::absolute(f).string() << "\"";
	}
	
	return executeCommandSafe(ossCmd.str());
}

bool Rar::compressDirectory(std::string const& directoryPath, std::string const& archiveName) const
{
	if(containsUnsafeChars(directoryPath) || containsUnsafeChars(archiveName))
		return false;
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	fs::path dirPath = fs::absolute(directoryPath);
	
	std::ostringstream ossCmd;
	
	#if defined(WINDOWS)
		ossCmd << "\"" << rar.path.string() << "\" a -r \"" << archivePath.string() << "\" \"" << dirPath.string() << "\\*\"";
	#else
		ossCmd << rar.path << " a -r \"" << archivePath.string() << "\" \"" << dirPath.string() << "/*\"";
	#endif
	
	return executeCommandSafe(ossCmd.str());
}


std::ostream& operator<<(std::ostream& os, Rar const& src)
{
	src.write(os);
	
	return os;
}


fs::path Rar::resolveArchivePath(std::string const& archiveName) const
{
	fs::path archivePath(archiveName);
	
	if(archivePath.is_absolute())
		return archivePath;
	else
		return fs::current_path() / archivePath;
}

bool Rar::executeCommandSafe(std::string const& command) const
{
	#if !defined(WINDOWS) && !defined(UNIX)
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

RarDetectionResult Rar::findRarExecutable() const
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
				m_cachedRar.type = RarType::WINRAR;
				m_cachedRar.path = fs::path(pathBuffer);
				
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
				m_cachedRar.type = RarType::WINRAR;
				m_cachedRar.path = fs::path(p);
				
				return m_cachedRar;
			}
		}
	#endif
	
	char const* envPath = std::getenv("PATH");
	
	if(envPath)
	{
		std::string pathEnv(envPath);
		size_t start = 0;
		size_t end = 0;
		
		while((end = pathEnv.find(PATH_SEPARATOR, start)) != std::string::npos)
		{
			fs::path dir = pathEnv.substr(start, end - start);
			
			if(fileExists(dir / "rar"))
			{
				m_cachedRar.found = true;
				m_cachedRar.type = RarType::RAR;
				m_cachedRar.path = dir / "rar";
				
				return m_cachedRar;
			}
			
			start = end + 1;
		}
		
		fs::path lastDir = pathEnv.substr(start);
		
		if(fileExists(lastDir / "rar"))
		{
			m_cachedRar.found = true;
			m_cachedRar.type = RarType::RAR;
			m_cachedRar.path = lastDir / "rar";
			
			return m_cachedRar;
		}
	}
	
	return m_cachedRar;
}

bool Rar::fileExists(fs::path const& path) const
{
	#if defined(WINDOWS)
		return fs::exists(path) || fs::exists(path.string() + ".exe");
	#elif defined(UNIX)
		return fs::exists(path) && access(path.c_str(), X_OK) == 0;
	#else
		return false;
	#endif
}

bool Rar::containsUnsafeChars(std::string const& str) const
{
	static std::string const unsafe = "\"';|&`$<>(){}\\!#~";
	
	return str.find_first_of(unsafe) != std::string::npos;
}


void Rar::write(std::ostream& os) const
{
	RarDetectionResult result = findRarExecutable();
	
	if(!result.found)
		os << "Rar / WinRAR not found. Please, install it.";
	else
	{
		os << "Rar / WinRAR found.\n"
		   << "Type : " << (result.type == RarType::RAR ? "rar" : "WinRAR") << "\n"
		   << "Path : " << result.path.string();
	}
}