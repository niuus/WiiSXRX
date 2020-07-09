/************************************************************************

Copyright mooby 2002
Copyright tehpola 2010

CDRMooby2 CDDAData.cpp
http://mooby.psxfanatics.com

  This file is protected by the GNU GPL which should be included with
  the source code distribution.

************************************************************************/

#include "CDDAData.hpp"

using namespace std;

extern std::string programName;

static sem_t audioReady;
#define REPEATALL 0
#define REPEATONE 1
#define PLAYONE   2
static int repeatPref = REPEATALL;

static void* CDDAThread(void* userData){
	return NULL;
}

// this callback repeats one track over and over
int CDDACallbackRepeat(  void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     /*PaTimestamp outTime,*/ void *userData )
{
   unsigned int i;
/* Cast data passed through stream to our structure type. */
   PlayCDDAData* data = (PlayCDDAData*)userData;
   short* out = (short*)outputBuffer;
    
   data->theCD->seek(data->CDDAPos);
   short* buffer = (short*)data->theCD->getBuffer();
   
   buffer += data->frameOffset;

   float volume = data->volume;

      // buffer the data
   for( i=0; i<framesPerBuffer; i++ )
   {
    /* Stereo channels are interleaved. */
      *out++ = (short)(*buffer++ * volume);              /* left */
      *out++ = (short)(*buffer++ * volume);             /* right */
      data->frameOffset += 4;

         // at the end of a frame, get the next one
      if (data->frameOffset == bytesPerFrame)
      {
         data->CDDAPos += CDTime(0,0,1);

            // when at the end of this track, loop to the start
            // of this track
         if (data->CDDAPos == data->CDDAEnd)
         {
            data->CDDAPos = data->CDDAStart;
         }

         data->theCD->seek(data->CDDAPos);
         data->frameOffset = 0;
         buffer = (short*)data->theCD->getBuffer();
      }
   }
   return 0;
}

// this callback plays through one track once and stops
int CDDACallbackOneTrackStop(  void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     /*PaTimestamp outTime,*/ void *userData )
{
   unsigned int i;
/* Cast data passed through stream to our structure type. */
   PlayCDDAData* data = (PlayCDDAData*)userData;
   short* out = (short*)outputBuffer;
   short* buffer;

      // seek to the current CDDA read position
   if (!data->endOfTrack)
   {
      data->theCD->seek(data->CDDAPos);
      buffer = (short*)data->theCD->getBuffer();
   }
   else
   {
      buffer = (short*)data->nullAudio;
   }

   buffer += data->frameOffset;

   float volume = data->volume;

      // buffer the data
   for( i=0; i<framesPerBuffer; i++ )
   {
    /* Stereo channels are interleaved. */
      *out++ = (short)(*buffer++ * volume);              /* left */
      *out++ = (short)(*buffer++ * volume);             /* right */
      data->frameOffset += 4;

         // at the end of a frame, get the next one
      if (data->frameOffset == bytesPerFrame)
      {
         data->CDDAPos += CDTime(0,0,1);

            // when at the end of this track, use null audio
         if (data->CDDAPos == data->CDDAEnd)
         {
            data->endOfTrack = true;
            buffer = (short*)data->nullAudio;
            data->CDDAPos -= CDTime(0,0,1);
            data->frameOffset = 0;
         }
            // not at end of track, just do normal buffering
         else
         {
            data->theCD->seek(data->CDDAPos);
            data->frameOffset = 0;
            buffer = (short*)data->theCD->getBuffer();
         }
      }
   }
   return 0;
}

PlayCDDAData::PlayCDDAData(const std::vector<TrackInfo> &ti, const CDTime &gapLength)
   : stream(NULL), volume(1),
     frameOffset(0), theCD(NULL), trackList(ti), playing(false),
     repeat(false), endOfTrack(false), pregapLength(gapLength)
{
   memset(nullAudio, 0, sizeof(nullAudio));
   
   live = true;
   LWP_SemInit(&audioReady, 1, 1);
   LWP_SemInit(&firstAudio, 0, 1);
   LWP_CreateThread(&audioThread, CDDAThread, this,
                    audioStack, audioStackSize, audioPriority);
}

PlayCDDAData::~PlayCDDAData()
{
	if (playing) stop(); 
	live = false;
	LWP_SemPost(firstAudio);
	LWP_SemDestroy(audioReady);
	LWP_SemDestroy(firstAudio);
	LWP_JoinThread(audioThread, NULL);
	delete theCD;
}

// initialize the CDDA file data and initalize the audio stream
void PlayCDDAData::openFile(const std::string& file, int type) 
{
   std::string extension;
   theCD = FileInterfaceFactory(file, extension, type);
   theCD->setPregap(pregapLength, trackList[2].trackStart);
      // disable extra caching on the file interface
   theCD->setCacheMode(FileInterface::oldMode);
}
   
// start playing the data
int PlayCDDAData::play(const CDTime& startTime)
{
   CDTime localStartTime = startTime;
   
      // if play was called with the same time as the previous call,
      // dont restart it.  this fixes a problem with FPSE's play call.
      // of course, if play is called with a different track, 
      // stop playing the current stream.
   if (playing)
   {
      if (startTime == InitialTime)
      {
         return 0;
      }
      else
      {
         stop();
      }
   }

   InitialTime = startTime;


      // figure out which track to play to set the end time
   if ( repeatPref != REPEATALL )
   {
      unsigned int i = 1;
      while ( (i < (trackList.size() - 1)) && (startTime > trackList[i].trackStart) )
      {
         i++;
      }
         // adjust the start time if it's blatantly off from the start time...
      if (localStartTime > trackList[i].trackStart)
      {
         if ( (localStartTime - trackList[i].trackStart) > CDTime(0,2,0))
         {
            localStartTime = trackList[i].trackStart;
         }
      }
      else
      {
         if ( (trackList[i].trackStart - localStartTime) > CDTime(0,2,0))
         {
            localStartTime = trackList[i].trackStart;
         }
      }
      CDDAStart = localStartTime;
      CDDAEnd = trackList[i].trackStart + trackList[i].trackLength;
   }
   else //repeatPref == REPEATALL
   {
      CDDAEnd = trackList[trackList.size() - 1].trackStart +
         trackList[trackList.size() - 1].trackLength;
      CDDAStart = trackList[2].trackStart;
      if (localStartTime > CDDAEnd)
      {
         localStartTime = CDDAStart;
      }
   }

         // set the cdda position, start and end times
   CDDAPos = localStartTime;

   endOfTrack = false;
   
   playing = true;

      // open a stream - pass in this CDDA object as the user data.
      // depending on the play mode, use a different callback
   LWP_SemPost(firstAudio);
   
   return 0;
}

// close the stream - nice and simple
int PlayCDDAData::stop()
{
   if (playing)
   {
      playing = false;
   }
   return 0;
}

