#ifndef DEF_UNRAR
#define DEF_UNRAR

#include <vector>
#include "Wrapper.hpp"

class Unrar : public Wrapper
{
	public:
		Unrar();
		Unrar(Unrar const& src);
		virtual ~Unrar() = default;
		
		Unrar& operator=(Unrar const& src);
		
		inline bool isUnrarInstalled() const { return findUnrarExecutable().found; }
		
		bool extractArchive(std::string const& archivePath, std::string const& outputPath = "", bool checkUnsafeChars = true) const;
		bool extractOneFile(std::string const& archivePath, std::string const& fileInArchive, std::string const& outputPath = "", bool checkUnsafeChars = true) const;
		bool listArchive(std::string const& archivePath, bool checkUnsafeChars = true) const;
		bool testArchive(std::string const& archivePath, bool checkUnsafeChars = true) const;
	
	private:
		mutable RarDetectionResult m_cachedUnrar;
		mutable bool m_unrarCached = false;
	
	protected:
		virtual RarDetectionResult findUnrarExecutable() const override;
};

#endif // DEF_UNRAR