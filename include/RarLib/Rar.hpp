#ifndef DEF_RAR
#define DEF_RAR

#include <vector>
#include "Wrapper.hpp"

class Rar : public Wrapper
{
	public:
		Rar();
		Rar(Rar const& src);
		virtual ~Rar() = default;
		
		Rar& operator=(Rar const& src);
		
		inline bool isRarInstalled() const { return findRarExecutable().found; }
		
		bool compressOneFile(std::string const& filePath, std::string const& archiveName) const;
		bool compressMultipleFiles(std::vector<std::string> const& files, std::string const& archiveName) const;
		bool compressDirectory(std::string const& directoryPath, std::string const& archiveName) const;
};

#endif // DEF_RAR