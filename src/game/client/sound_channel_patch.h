#ifndef SOUND_CHANNEL_PATCH_H
#define SOUND_CHANNEL_PATCH_H

#if defined( _WIN64 ) && defined( HL2_CLIENT_DLL ) && !defined( HL2MP )
// Keep Win32's CreateEvent macro out of the game event interface.
#pragma push_macro( "CreateEvent" )
#include "winlite.h"
#pragma pop_macro( "CreateEvent" )
#include "tier1/checksum_crc.h"

// Called only by CHLClient::Init, before Host_Init calls S_Init. Never patch from
// a sound callback: allocation can run on the audio thread once S_Init finishes.
static bool PatchDelayedSoundChannels()
{
	byte *pEngine = (byte *)GetModuleHandleA( "engine.dll" );
	if ( !pEngine )
		return false;

	const IMAGE_DOS_HEADER *pDos = (const IMAGE_DOS_HEADER *)pEngine;
	if ( pDos->e_magic != IMAGE_DOS_SIGNATURE || pDos->e_lfanew <= 0 || pDos->e_lfanew > 0x1000 )
		return false;
	const IMAGE_NT_HEADERS64 *pNt = (const IMAGE_NT_HEADERS64 *)( pEngine + pDos->e_lfanew );
	// HL2DM x64 engine SHA256:
	// 8cc17bcd7b75598a68c61c77b5863b8017ab7f6097a242a615353cc2c1974d05
	if ( pNt->Signature != IMAGE_NT_SIGNATURE || pNt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		 pNt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
		 pNt->FileHeader.TimeDateStamp != 0x6a0dec1b || pNt->OptionalHeader.SizeOfImage != 0x820000 )
	{
		Warning( "[sound-channel-patch] Unsupported engine build!!! Retaining player-voice workaround.\n" );
		return false;
	}

	// SND_StealDynamicChannel, RVA 0x2a330. Instead of collecting delayed exact
	// matches (and later killing/reusing them), fall through to normal protection
	// and free-slot allocation at 0x2a44f. Non-delayed replacement is unchanged.
	const byte original[] = { 0x41, 0xff, 0xc1, 0x89, 0x30 };
	const byte patched[] = { 0xe9, 0x13, 0x00, 0x00, 0x00 };
	byte *pPatch = pEngine + 0x2a437;
	bool bAlreadyPatched = memcmp( pPatch, patched, sizeof( patched ) ) == 0;
	byte allocator[0x6ef];
	memcpy( allocator, pEngine + 0x2a330, sizeof( allocator ) );
	if ( bAlreadyPatched )
		memcpy( allocator + 0x107, original, sizeof( original ) );
	if ( CRC32_ProcessSingleBuffer( allocator, sizeof( allocator ) ) != 0xad8e4501 ||
		 ( !bAlreadyPatched && memcmp( pPatch, original, sizeof( original ) ) != 0 ) )
	{
		Warning( "[sound-channel-patch] Allocator code mismatch. Retaining player-voice workaround.\n" );
		return false;
	}
	if ( bAlreadyPatched )
	{
		Msg( "[sound-channel-patch] Delayed channel protection already installed.\n" );
		return true;
	}

	// snd_initialized, checked/set by S_Init at 0x2be50/0x2bf1f. Also refuse
	// a late/reordered initialization rather than racing live sound allocation.
	if ( pEngine[0x4a55c0] )
	{
		Warning( "[sound-channel-patch] Sound already initialized. Retaining player-voice workaround.\n" );
		return false;
	}

	DWORD oldProtection;
	if ( !VirtualProtect( pPatch, sizeof( patched ), PAGE_EXECUTE_READWRITE, &oldProtection ) )
	{
		Warning( "[sound-channel-patch] VirtualProtect failed (%lu). Retaining player-voice workaround.\n", GetLastError() );
		return false;
	}
	memcpy( pPatch, patched, sizeof( patched ) );
	DWORD unused;
	if ( !VirtualProtect( pPatch, sizeof( patched ), oldProtection, &unused ) )
		Error( "[sound-channel-patch] Cannot restore engine page protection (%lu).\n", GetLastError() );
	if ( !FlushInstructionCache( GetCurrentProcess(), pPatch, sizeof( patched ) ) )
		Error( "[sound-channel-patch] Cannot flush engine instruction cache (%lu).\n", GetLastError() );
	Msg( "[sound-channel-patch] Installed delayed channel protection (HL2DM x64, engine+0x2a437). Player-voice workaround bypassed.\n" );
	return true;
}
#else
static bool PatchDelayedSoundChannels() { return false; }
#endif

#endif // SOUND_CHANNEL_PATCH_H
