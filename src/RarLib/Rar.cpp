#include "../../include/RarLib/Rar.hpp"

#include <sstream>
#include <cstdlib>
#include "../../include/RarLib/OS.hpp"

namespace fs = std::filesystem;

Rar::Rar() : Wrapper() {}

Rar::Rar(Rar const& src) : Wrapper(src) {}


Rar& Rar::operator=(Rar const& src)
{
	Wrapper::operator=(src);
	
	return *this;
}


bool Rar::compressOneFile(std::string const& filePath, std::string const& archiveName, bool checkUnsafeChars) const
{
	if(checkUnsafeChars)
	{
		if(containsUnsafeChars(filePath) || containsUnsafeChars(archiveName))
			return false;
	}
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	fs::path inputPath = fs::absolute(fs::u8path(filePath));
	
	#if defined(WINDOWS)
		std::wostringstream wossCmd;
		
		wossCmd << "\"" << rar.path.wstring() << "\" a -r \"" << archivePath.wstring() << "\" \"" << inputPath.wstring() << "\"";
		
		return executeCommandSafe(wossCmd.str());
	#else
		std::ostringstream ossCmd;
		
		ossCmd << rar.path << " a -r \"" << archivePath.u8string() << "\" \"" << inputPath.u8string() << "\"";
		
		return executeCommandSafe(ossCmd.str());
	#endif
}

bool Rar::compressMultipleFiles(std::vector<std::string> const& files, std::string const& archiveName) const
{
	if(containsUnsafeChars(archiveName))
		return false;
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	
	#if defined(WINDOWS)
		std::wostringstream wossCmd;
		
		wossCmd << "\"" << rar.path.wstring() << "\" a -r \"" << archivePath.wstring() << "\"";
		
		for(std::string const& f : files)
		{
			if(containsUnsafeChars(f))
				return false;
			
			wossCmd << " \"" << fs::absolute(fs::u8path(f)).wstring() << "\"";
		}
		
		return executeCommandSafe(wossCmd.str());
	#else
		std::ostringstream ossCmd;
		
		ossCmd << rar.path << " a -r \"" << archivePath.u8string() << "\"";
		
		for(std::string const& f : files)
		{
			if(containsUnsafeChars(f))
				return false;
			
			ossCmd << " \"" << fs::absolute(fs::u8path(f)).u8string() << "\"";
		}
		
		return executeCommandSafe(ossCmd.str());
	#endif
}

bool Rar::compressDirectory(std::string const& directoryPath, std::string const& archiveName, bool checkUnsafeChars) const
{
	if(checkUnsafeChars)
	{
		if(containsUnsafeChars(directoryPath) || containsUnsafeChars(archiveName))
			return false;
	}
	
	RarDetectionResult const& rar = findRarExecutable();
	
	if(!rar.found)
		return false;
	
	fs::path archivePath = resolveArchivePath(archiveName);
	fs::path dirPath = fs::absolute(fs::u8path(directoryPath));
	
	#if defined(WINDOWS)
		std::wostringstream wossCmd;
		
		wossCmd << "\"" << rar.path.wstring() << "\" a -r \"" << archivePath.wstring() << "\" \"" << dirPath.wstring() << "\\*\"";
		
		return executeCommandSafe(wossCmd.str());
	#else
		std::ostringstream ossCmd;
		
		ossCmd << rar.path << " a -r \"" << archivePath.u8string() << "\" \"" << dirPath.u8string() << "/*\"";
		
		return executeCommandSafe(ossCmd.str());
	#endif
}