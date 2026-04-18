#ifndef DEF_RAR
#define DEF_RAR

#include <ostream>
#include <filesystem>
#include <string>
#include <vector>

enum RarType
{
	WINRAR,
	RAR,
	EMPTY
};

struct RarDetectionResult
{
	bool found = false;
	RarType type;
	std::filesystem::path path;
};

typedef struct RarDetectionResult RarDetectionResult;

class Rar
{
	public:
		Rar();
		Rar(Rar const& src);
		~Rar();
		
		Rar& operator=(Rar const& src);
		
		inline bool isRarInstalled() const { return findRarExecutable().found; }
		
		bool compressOneFile(std::string const& filePath, std::string const& archiveName) const;
		bool compressMultipleFiles(std::vector<std::string> const& files, std::string const& archiveName) const;
		bool compressDirectory(std::string const& directoryPath, std::string const& archiveName) const;
		
		friend std::ostream& operator<<(std::ostream& os, Rar const& src);
	
	private:
		std::filesystem::path resolveArchivePath(std::string const& archiveName) const;
		bool executeCommandSafe(std::string const& command) const;
		RarDetectionResult findRarExecutable() const;
		bool fileExists(std::filesystem::path const& path) const;
	
	protected:
		virtual void write(std::ostream& os) const;
};

#endif // DEF_RAR