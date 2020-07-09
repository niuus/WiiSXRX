/************************************************************************

Copyright mooby 2002

CDRMooby2 utils.hpp
http://mooby.psxfanatics.com

  This file is protected by the GNU GPL which should be included with
  the source code distribution.

************************************************************************/

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>

/* 
   This file is for any utility function or constant that has use outside the scope of any
   single file
*/

// specifies how you want the MSF time formatted
// msf/fsm -> if time is specified in minute, second, frame or backwards
// int/bcd -> integer or binary coded decimal format
enum TDTNFormat
{
   msfint,
   fsmint,
   fsmbcd,
   msfbcd
};

// a quick way to tell which API set is being used
enum EMUMode
{
   psemu,
   fpse
};

// converts uchar in c to BCD character
#define intToBCD(c) (unsigned char)((c%10) | ((c/10)<<4))

// converts BCD number in c to uchar
#define BCDToInt(c) (unsigned char)((c & 0x0F) + 10 * ((c & 0xF0) >> 4))

static const std::string theUsualSuspects = 
  "Common image files (*.{bz,bz.index,Z,Z.table,bin,bwi,img,iso,rar})";

// CD time constants
static const unsigned long secondsPerMinute = 60;
static const unsigned long framesPerSecond = 75;
static const unsigned long framesPerMinute = framesPerSecond * secondsPerMinute;
static const unsigned long bytesPerFrame = 2352;
static const unsigned long bytesPerSecond = bytesPerFrame * framesPerSecond;
static const unsigned long bytesPerMinute = bytesPerSecond * secondsPerMinute;

extern "C" {
  void SysPrintf(const char *fmt, ...);
}

inline void moobyMessage(const std::string& message)
{
  SysPrintf("%s\r\n",message.c_str());
}

inline void moobyMessage(const char* message)
{
  SysPrintf("%s\r\n",message);
}

inline char* moobyFileChooser(const char* message, const char* filespec, 
                              const std::string& last = std::string())
{
  return NULL;
}

inline int moobyAsk(const char* message)
{
  return 0;
}

// since most binary data is stored little endian, this flips
// the data bits for big endian systems
template <class T>
inline void flipBits(T& num)
{
   char* front = (char*)&num;
   char* end = front + sizeof(T) - 1;
   while (front < end)
   {
      char t;
      t = *front;
      *front = *end;
      *end = t;
      front++;
      end--;
   }
}

// given a string and a number, returns the ' ' delimited word
// at position num
inline std::string word(const std::string& str, const unsigned long num)
{
   if (str == std::string())
      return str;

   unsigned long i;
   std::string::size_type startpos = 0;
   std::string::size_type endpos = 0;

   for(i = 0; i < num; i++)
   {
      startpos = str.find_first_not_of(' ', endpos);
      if (startpos == std::string::npos)
         return std::string();

      endpos = str.find_first_of(' ', startpos);
      if (endpos == std::string::npos)
      {
         endpos = str.size();
      }
   }
   return str.substr(startpos, endpos - startpos);
}

// turns a string into its lowercase...
inline std::string tolcstr(const std::string& s)
{
   std::string::size_type i;
   std::string toReturn(s);
   for (i = 0; i < s.size(); i++)
      toReturn[i] = tolower(s[i]);
   return toReturn;
}

// tries to match the extension ext to the end of the file name
inline bool extensionMatches(const std::string& file, const std::string& ext)
{
   if (file.size() < ext.size()) return false;
   return (tolcstr(file.substr(file.size() - ext.size())) == ext);
}

// returns what the name of the program running this plugin is.
inline std::string getProgramName(void)
{
   //std::string toReturn;
   return "WiiSXRX";
}

#endif
