#include "../../include/RarLib/Unrar.hpp"

#include <filesystem>
#include <sstream>
#include "../../include/RarLib/OS.hpp"

namespace fs = std::filesystem;

Unrar::Unrar() : Wrapper() {}

Unrar::Unrar(Unrar const& src) : Wrapper(src), m_cachedUnrar(src.m_cachedUnrar), m_unrarCached(src.m_unrarCached) {}


Unrar& Unrar::operator=(Unrar const& src)
{
	Wrapper::operator=(src);
	
	if(this != &src)
	{
		m_cachedUnrar = src.m_cachedUnrar;
		m_unrarCached = src.m_unrarCached;
	}
	
	return *this;
}


bool Unrar::extractArchive(std::string const& archivePath, std::string const& outputPath) const
{
	if(containsUnsafeChars(archivePath) || containsUnsafeChars(outputPath))
		return false;
	
	RarDetectionResult const& extractor = findExtractionExecutable();
	
	if(!extractor.found)
		return false;
	
	fs::path archiveAbs = fs::absolute(archivePath);
	
	// Resolve output directory — default to current working directory
	fs::path outputDir = outputPath.empty() ? fs::current_path() : fs::absolute(outputPath);
	
	std::ostringstream ossCmd;
	
	// Flag 'x' preserves the full directory structure from the archive.
	// Flag '-o+' overwrites existing files without prompting (WinRAR / rar).
	// unrar uses the same flags.
	#if defined(WINDOWS)
		ossCmd << "\"" << extractor.path.string() << "\" x -o+ \""
			   << archiveAbs.string() << "\" \""
			   << outputDir.string() << "\\\"";
	#else
		ossCmd << extractor.path << " x -o+ \""
			   << archiveAbs.string() << "\" \""
			   << outputDir.string() << "/\"";
	#endif
	
	return executeCommandSafe(ossCmd.str());
}

bool Unrar::extractOneFile(std::string const& archivePath, std::string const& fileInArchive, std::string const& outputPath) const
{
	if(containsUnsafeChars(archivePath) || containsUnsafeChars(fileInArchive) || containsUnsafeChars(outputPath))
		return false;
	
	RarDetectionResult const& extractor = findExtractionExecutable();
	
	if(!extractor.found)
		return false;
	
	fs::path archiveAbs = fs::absolute(archivePath);
	
	fs::path outputDir = outputPath.empty() ? fs::current_path() : fs::absolute(outputPath);
	
	std::ostringstream ossCmd;
	
	// Flag 'e' extracts without recreating the directory structure.
	// The specific file to extract is passed as the third argument.
	#if defined(WINDOWS)
		ossCmd << "\"" << extractor.path.string() << "\" e -o+ \""
			   << archiveAbs.string() << "\" \""
			   << fileInArchive << "\" \""
			   << outputDir.string() << "\\\"";
	#else
		ossCmd << extractor.path << " e -o+ \""
			   << archiveAbs.string() << "\" \""
			   << fileInArchive << "\" \""
			   << outputDir.string() << "/\"";
	#endif
	
	return executeCommandSafe(ossCmd.str());
}

bool Unrar::listArchive(std::string const& archivePath) const
{
	if(containsUnsafeChars(archivePath))
		return false;
	
	RarDetectionResult const& extractor = findExtractionExecutable();
	
	if(!extractor.found)
		return false;
	
	fs::path archiveAbs = fs::absolute(archivePath);
	
	std::ostringstream ossCmd;
	
	// Flag 'l' lists archive contents with technical details (size, date, attributes).
	#if defined(WINDOWS)
		ossCmd << "\"" << extractor.path.string() << "\" l \"" << archiveAbs.string() << "\"";
	#else
		ossCmd << extractor.path << " l \"" << archiveAbs.string() << "\"";
	#endif
	
	// Output must be visible to the caller — use executeCommandWithOutput instead of executeCommandSafe.
	return executeCommandWithOutput(ossCmd.str());
}

bool Unrar::testArchive(std::string const& archivePath) const
{
	if(containsUnsafeChars(archivePath))
		return false;
	
	RarDetectionResult const& extractor = findExtractionExecutable();
	
	if(!extractor.found)
		return false;
	
	fs::path archiveAbs = fs::absolute(archivePath);
	
	std::ostringstream ossCmd;
	
	// Flag 't' tests the archive integrity (CRC checks on every file).
	#if defined(WINDOWS)
		ossCmd << "\"" << extractor.path.string() << "\" t \"" << archiveAbs.string() << "\"";
	#else
		ossCmd << extractor.path << " t \"" << archiveAbs.string() << "\"";
	#endif
	
	return executeCommandSafe(ossCmd.str());
}


RarDetectionResult Unrar::findUnrarExecutable() const
{
	if(m_unrarCached)
		return m_cachedUnrar;
	
	m_unrarCached = true;
	m_cachedUnrar = Wrapper::findUnrarExecutable();
	
	return m_cachedUnrar;
}