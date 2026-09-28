// This DLL loads after materialsystem sets this value but before client actually sizes the constant arrays, so you can literally just say "actually, nevermind" and change the constants back to normal.
#include <windows.h>
#include "tier1/interface.h"
#include "shaderapi/IShaderDevice.h"

static struct PsConstPatch_t
{
	PsConstPatch_t()
	{
		HMODULE hApi = GetModuleHandleA( "shaderapidx9.dll" );
		CreateInterfaceFn pfn = hApi ? (CreateInterfaceFn)GetProcAddress( hApi, "CreateInterface" ) : NULL;
		char **pMgr = pfn ? (char **)pfn( SHADER_DEVICE_MGR_INTERFACE_VERSION, NULL ) : NULL;
		for ( int s = 0; pMgr && s < 0x400 / (int)sizeof( void * ); ++s )
		{
			MEMORY_BASIC_INFORMATION mbi;
			if ( !VirtualQuery( pMgr[s], &mbi, sizeof( mbi ) ) || mbi.State != MEM_COMMIT || ( mbi.Protect & 0xFF ) != PAGE_READWRITE )
				continue;
			const int nInts = (int)( ( (char *)mbi.BaseAddress + mbi.RegionSize - pMgr[s] ) / 4 ) - 6;
			int *p = (int *)pMgr[s];
			for ( int i = 0; i < nInts && i < 0x4000; ++i )
				if ( p[i] == 32 && p[i+1] == 16 && p[i+2] == 16 && p[i+3] == 256 && p[i+4] == 16 && p[i+5] == 16 )
					p[i] = 224;
		}
	}
} s_PsConstPatch;
