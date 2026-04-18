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