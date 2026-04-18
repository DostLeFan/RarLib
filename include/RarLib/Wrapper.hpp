#ifndef DEF_WRAPPER
#define DEF_WRAPPER

#include <ostream>
#include <string>
#include <filesystem>

enum RarType
{
	WINRAR,
	RAR,
	UNRAR,
	EMPTY
};

struct RarDetectionResult
{
	bool found = false;
	RarType type;
	std::filesystem::path path;
};

typedef struct RarDetectionResult RarDetectionResult;

class Wrapper
{
	public:
		Wrapper();
		Wrapper(Wrapper const& src);
		virtual ~Wrapper() = default;
		
		Wrapper& operator=(Wrapper const& src);
		
		friend std::ostream& operator<<(std::ostream& os, Wrapper const& src);
	
	private:
		mutable RarDetectionResult m_cachedRar;
		mutable bool m_rarCached = false;
	
	protected:
		virtual void write(std::ostream& os) const;
		
		std::filesystem::path resolveArchivePath(std::string const& archiveName) const;
		bool executeCommandSafe(std::string const& command) const;
		bool executeCommandWithOutput(std::string const& command) const;
		bool fileExists(std::filesystem::path const& path) const;
		bool containsUnsafeChars(std::string const& str) const;
		RarDetectionResult findRarExecutable() const;
		virtual RarDetectionResult findUnrarExecutable() const;
		RarDetectionResult findExtractionExecutable() const;
};

#endif // DEF_WRAPPER