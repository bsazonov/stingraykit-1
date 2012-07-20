#include <stingray/toolkit/Factory.h>

#include <stingray/app/ApplicationContextPublic.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/Scheduler.h>
#include <stingray/app/zapper/User.h>
#include <stingray/mpeg/Stream.h>
#include <stingray/parentalcontrol/AgeRating.h>
#ifdef PLATFORM_EMU
#	include <stingray/platform/emu/scanner/Channel.h>
#endif
#ifdef PLATFORM_EMU
#	include <stingray/platform/emu/scanner/Channel.h>
#endif
#ifdef PLATFORM_EMU
#	include <stingray/platform/emu/scanner/Channel.h>
#endif
#ifdef PLATFORM_EMU
#	include <stingray/platform/emu/scanner/Channel.h>
#endif
#include <stingray/records/FileSystemRecord.h>
#include <stingray/scanner/DVBServiceId.h>
#include <stingray/scanner/DefaultDVBTBandInfo.h>
#include <stingray/scanner/DefaultMpegService.h>
#include <stingray/scanner/DefaultMpegService.h>
#include <stingray/scanner/DefaultMpegService.h>
#include <stingray/scanner/DefaultMpegService.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultMpegStreamDescriptor.h>
#include <stingray/scanner/DefaultScanParams.h>
#include <stingray/scanner/DefaultScanResult.h>
#include <stingray/scanner/DefaultServiceNetworkInfo.h>
#include <stingray/scanner/LcnList.h>
#include <stingray/scanner/OtherTransportInfoEntry.h>
#include <stingray/storage/FatFileSystemProber.h>
#include <stingray/streams/RecordStreamMetaInfo.h>
#include <stingray/tuners/DefaultDVBTTransport.h>
#include <stingray/tuners/dvbs/Antenna.h>
#include <stingray/tuners/dvbs/Satellite.h>
#include <stingray/tuners/dvbs/Transport.h>

/* WARNING! This is autogenerated file, DO NOT EDIT! */

namespace stingray { namespace Detail
{
	void Factory::RegisterTypes()
	{
#ifdef BUILD_SHARED_LIB
		/*nothing*/
#else
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::AppChannel);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::Alarm);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::ContinuousScheduledEvent);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::DeferredStandby);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::InfiniteScheduledEvent);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::ScheduledEvent);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::ScheduledRecord);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::ScheduledViewing);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(app::User);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(mpeg::Stream);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(AgeRating);
#ifdef PLATFORM_EMU
		TOOLKIT_REGISTER_CLASS_EXPLICIT(emu::EmuServiceId);
#endif
#ifdef PLATFORM_EMU
		TOOLKIT_REGISTER_CLASS_EXPLICIT(emu::RadioChannel);
#endif
#ifdef PLATFORM_EMU
		TOOLKIT_REGISTER_CLASS_EXPLICIT(emu::StreamDescriptor);
#endif
#ifdef PLATFORM_EMU
		TOOLKIT_REGISTER_CLASS_EXPLICIT(emu::TVChannel);
#endif
		TOOLKIT_REGISTER_CLASS_EXPLICIT(FileSystemRecord);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DVBServiceId);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultDVBTBandInfo);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultMpegChannel);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultMpegRadioChannel);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultMpegService);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultMpegTVChannel);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultMpegStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegAudioStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegPcrStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegSubtitlesStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegTeletextBasedSubtitlesStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegTeletextStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(MpegVideoStreamDescriptor);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultScanParams);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultScanResult);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultServiceNetworkInfo);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(LcnList);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(OtherTransportInfoEntry);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(FatFileSystemIdentity);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(RecordStreamMetaInfo);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(DefaultDVBTTransport);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(dvbs::Antenna);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(dvbs::Satellite);
		TOOLKIT_REGISTER_CLASS_EXPLICIT(dvbs::Transport);
#endif
	}
}}
