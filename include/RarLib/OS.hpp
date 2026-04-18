#ifndef OS_HPP
#define OS_HPP

// Thanks to this site which define a lot of pre-defined compiler macros : https://sourceforge.net/p/predef/wiki/Home/

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(_WIN16) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || defined(_WIN32_WCE) || defined(__MINGW32__) || defined(__MINGW64__)
	#ifndef WINDOWS
		#define WINDOWS
	#endif
#elif defined(__unix__) || defined(__unix) || defined(_POSIX_VERSION) \
	  (defined(__APPLE__) && defined(__MACH__)) || defined(macintosh) || defined(Macintosh) \
	  || defined(__linux__) || defined(__gnu_linux__) \
	  || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) \
	  || defined(_AIX) || defined(__hpux) || defined(__sun) \
	  || defined(__SVR4) || defined(__svr4__)
	#ifndef UNIX
		#define UNIX
	#endif
#endif

#endif // OS_HPP