

#include <windows.h>
#include <stdio.h>

#include "STD_Windows.h"
#include "STD_Debug.h"
#include "STD_File.h"

#include "d3d_draw.h"
#include "d3d_main.h"


#define FULL_FLIP	D3DSWAPEFFECT_FLIP



D3D_MAIN				D3DMain = {FALSE, FALSE, FALSE, FALSE, FALSE};

LPDIRECT3D8				pD3D = NULL;
LPDIRECT3DDEVICE8		pD3DDevice = NULL;
D3DPRESENT_PARAMETERS	d3dppApp;

D3DCAPS_STRUCT			D3DCapsStruct;



int		WinX;
int		WinY;

void D3DM_SetWindowXY( int x, int y )
{
	WinX = x;
	WinY = y;
}

int		WinHeight;
int		WinWidth;


DWORD FullModeStyle = (WS_POPUP | WS_CLIPCHILDREN | WS_SYSMENU | WS_VISIBLE);
DWORD WindowStyle = ((WS_POPUP | WS_CLIPCHILDREN | WS_SYSMENU | WS_VISIBLE) | WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX);

void D3DM_SetWindowStyle( DWORD win, DWORD full )
{
	WindowStyle = win;
	FullModeStyle = full;
}




int	IsTextureFormatOk( D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat )
{
	int	ret=0;
	HRESULT hr;
	hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
											D3DDEVTYPE_HAL,
											AdapterFormat,
											0,
											D3DRTYPE_TEXTURE,
											TextureFormat );
	if(D3D_OK == hr){
		ret=1;
	}

	hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
											D3DDEVTYPE_HAL,
											AdapterFormat,
											D3DUSAGE_RENDERTARGET,
											D3DRTYPE_TEXTURE,
											TextureFormat );
	if(D3D_OK == hr){
		ret=2;
	}

	return ret;
}

#define D3DFMT_D3DD_MAX		(D3DFMT_P8+2)
static char *TxFmtMode[D3DFMT_D3DD_MAX]={
	"D3DFMT_UNKNOWN",NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,

	NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,

	"D3DFMT_R8G8B8","D3DFMT_A8R8G8B8","D3DFMT_X8R8G8B8","D3DFMT_R5G6B5","D3DFMT_X1R5G5B5",
	"D3DFMT_A1R5G5B5","D3DFMT_A4R4G4B4","D3DFMT_R3G3B2","D3DFMT_A8","D3DFMT_A8R3G3B2",

	"D3DFMT_X4R4G4B4","D3DFMT_A2B10G10R10",NULL,NULL,"D3DFMT_G16R16",
	NULL,NULL,NULL,NULL,NULL,

	"D3DFMT_A8P8","D3DFMT_P8","D3DFMT_ERR",
};

void EnumAdapters( int nAdapter )
{
	int		i,n;
	char	buf[128];

	n = pD3D->GetAdapterModeCount(nAdapter);
	D3DCapsStruct.m_DisplayModeNum = n;

	if( D3DCapsStruct.m_DisplayMode==NULL )
		D3DCapsStruct.m_DisplayMode = (D3DDISPLAYMODE*)GAlloc( sizeof(D3DDISPLAYMODE)*n );

	for( i=0 ; i<n ; i++ ){
		if( pD3D->EnumAdapterModes(nAdapter,i,&D3DCapsStruct.m_DisplayMode[i]) == D3D_OK){
			wsprintf( buf, "%d x %d (%d Hz) - %s\n",
							D3DCapsStruct.m_DisplayMode[i].Width,
							D3DCapsStruct.m_DisplayMode[i].Height,
							D3DCapsStruct.m_DisplayMode[i].RefreshRate,
							TxFmtMode[ LIM(D3DCapsStruct.m_DisplayMode[i].Format,0,D3DFMT_D3DD_MAX-1) ] );
			DebugPrintf( buf );
		}
	}
}



int D3DM_InitDevices( HWND hwnd, int w, int h, int full, HMENU menu )
{
	char	buf[256];
	int		i,j;
		LPDIRECTDRAW7 pDD = NULL;	
		HRESULT hr;
		if( FAILED( hr = DirectDrawCreateEx( NULL, (VOID**)&pDD, IID_IDirectDraw7, NULL ) ) )
			return DDENUMRET_CANCEL;

		D3DCapsStruct.m_ddCaps.dwSize = sizeof(DDCAPS);
		pDD->GetCaps( &D3DCapsStruct.m_ddCaps, NULL );
		if(pDD) {
			pDD->Release();
			pDD = NULL;
		}
	ZeroMemory( d3dTexture, sizeof(D3DD_TEXTURE)*D3DD_MAX_TEXTURE_AMOUNT );
	ZeroMemory( d3dText, sizeof(D3DD_TEXT)*D3DD_MAX_TEXT_AMOUNT );
	ZeroMemory( d3dDisp, sizeof(D3DD_DISP)*D3DD_MAX_DISP_AMOUNT );


	
	pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if(pD3D == NULL){
		MessageBox(NULL,"Direct3D�I�u�W�F�N�g�̐����Ɏ��s���܂����B[DirectX8.1�������Ă��Ȃ��H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	
	if( FAILED(pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCapsStruct.m_d3dCaps)) ){
		MessageBox(NULL,"�f�o�C�X�\�͂̎擾�Ɏ��s���܂���","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
		return FALSE;
	}
	
	EnumAdapters(D3DADAPTER_DEFAULT);

	
	if( (int)D3DCapsStruct.m_d3dCaps.MaxTextureWidth < D3DD_TEXTURE_CONTROL_SIZE ){
		DebugBox( NULL, "���̃r�f�I�J�[�h�́A�� %d pixel �ȏ�̃T�C�Y�̃e�N�X�`���𐶐��ł��܂���B[%s]", D3DD_TEXTURE_CONTROL_SIZE);
		return FALSE;
	}else if( (int)D3DCapsStruct.m_d3dCaps.MaxTextureHeight < D3DD_TEXTURE_CONTROL_SIZE ){
		DebugBox( NULL, "���̃r�f�I�J�[�h�́A���� %d pixel �ȏ�̃T�C�Y�̃e�N�X�`���𐶐��ł��܂���B[%s]", D3DD_TEXTURE_CONTROL_SIZE );
		return FALSE;
	}

	if( !(D3DCapsStruct.m_d3dCaps.ShadeCaps&D3DPSHADECAPS_ALPHAGOURAUDBLEND) ){
		MessageBox(NULL,"���̃r�f�I�f�o�C�X�̓O�[���u�����f�B���O�ɑΉ����Ă��܂���B\n�Q�[���̉摜������邱�Ƃ�����܂�","�x��", MB_OK | MB_ICONSTOP);
	}
	if( !(D3DCapsStruct.m_d3dCaps.ShadeCaps&D3DPSHADECAPS_COLORGOURAUDRGB) ){
		MessageBox(NULL,"���̃r�f�I�f�o�C�X�̓O�[���V�F�[�f�B���O�ɑΉ����Ă��܂���B\n�Q�[���̉摜������邱�Ƃ�����܂�","�x��", MB_OK | MB_ICONSTOP);
	}

	if( D3DCapsStruct.m_d3dCaps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY ){
		DebugBox( NULL, "���̃r�f�I�J�[�h�͒����`�e�N�X�`���𐶐��ł��܂���B[�f�o�b�O�p�_�C�A���O]" );
	}

	
	if( FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&D3DCapsStruct.m_NowDisplayMode)) ){	
		MessageBox(NULL,"�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂����B[�Ȃɂ䂦�H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
		return FALSE;
	}
	D3DCapsStruct.m_WindowDisplayMode = D3DCapsStruct.m_NowDisplayMode;
	D3DMain.m_DrawHwnd = hwnd;
	D3DMain.m_MenuHwnd = menu;
	if( GetSystemMetrics(SM_CXFULLSCREEN)<=800 || GetSystemMetrics(SM_CYFULLSCREEN)<=600){
		D3DMain.m_FullScreenOnly = TRUE;
		D3DMain.m_WindowMode = FALSE;
	}else{
		D3DMain.m_FullScreenOnly = FALSE;
		D3DMain.m_WindowMode = !full;
	}


	ZeroMemory(&d3dppApp,sizeof(d3dppApp));
	


	WinWidth  = w;
	WinHeight = h;



	d3dppApp.SwapEffect = D3DSWAPEFFECT_COPY;

	d3dppApp.BackBufferFormat = D3DCapsStruct.m_NowDisplayMode.Format;
	d3dppApp.BackBufferCount = 1;
	d3dppApp.BackBufferWidth  = WinWidth;
	d3dppApp.BackBufferHeight = WinHeight;





	d3dppApp.Windowed = TRUE;							
	d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	
	HRESULT	ret;
	ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
	if( FAILED(ret) ){	
		ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
		if( FAILED(ret) ){	
			D3DMain.m_FullScreenOnly = TRUE;
		}
	}
	if(D3DMain.m_FullScreenOnly==TRUE){
		RELEASE_3D(pD3DDevice);

		D3DMain.m_WindowMode = FALSE;
		ZeroMemory(&d3dppApp,sizeof(d3dppApp));
		for(i=0;i<D3DCapsStruct.m_DisplayModeNum;i++){
			if( D3DCapsStruct.m_DisplayMode[i].Width  == 800 && D3DCapsStruct.m_DisplayMode[i].Height == 600 ){
				switch( D3DCapsStruct.m_DisplayMode[i].Format ){
					case D3DFMT_R5G6B5:		case D3DFMT_X1R5G5B5:
					case D3DFMT_A1R5G5B5:	case D3DFMT_A4R4G4B4:
					case D3DFMT_X4R4G4B4:
						D3DCapsStruct.m_FullModeNum=i;

						d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
						d3dppApp.Windowed = D3DMain.m_WindowMode;		
						d3dppApp.SwapEffect = FULL_FLIP;		
						d3dppApp.BackBufferFormat = D3DCapsStruct.m_DisplayMode[i].Format;	
						d3dppApp.BackBufferCount = 1;
						d3dppApp.BackBufferWidth  = WinWidth;
						d3dppApp.BackBufferHeight = WinHeight;



						break;
				}
			}
			if(d3dppApp.SwapEffect) break;
		}
		if(!full){
			wsprintf( buf, "���̃r�f�I�J�[�h�̌��݂̃��[�h�ł̓Q�[�������s�ł��܂���B�t���X�N���[�������܂����H\n[%d]", D3DCapsStruct.m_DisplayMode[i].RefreshRate );
			if( MessageBox( NULL, buf, "�₢���킹", MB_YESNO )==IDNO ){
				RELEASE_3D(pD3D);
				return FALSE;
			}
		}

		ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
		if( FAILED(ret) ){	
			ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
			if( FAILED(ret) ){	
				switch(ret){
				default:
				case D3DERR_OUTOFVIDEOMEMORY:
					DebugBox( NULL, "Direct3D ���������s���̂ɏ\���ȃf�B�X�v���C ������������܂���B" );
					RELEASE_3D(pD3D);
					return FALSE;
				case D3DERR_INVALIDCALL:
					DebugBox( NULL, "Direct3D�̏������Ɏ��s���܂���[D3DERR_INVALIDCALL]\n���̃O���t�B�b�N�J�[�h�͕K�v�ȋ@�\���T�|�[�g���Ă��Ȃ����A\n���邢��DirectX8�ɑΉ������h���C�o�������Ă��܂���B" );
					RELEASE_3D(pD3D);
					return FALSE;
				case D3DERR_NOTAVAILABLE:
					DebugBox( NULL, "Direct3D�̏������Ɏ��s���܂���[D3DERR_NOTAVAILABLE]\n���̃O���t�B�b�N�J�[�h�͕K�v�ȋ@�\���T�|�[�g���Ă��Ȃ����A\n���邢��DirectX8�ɑΉ������h���C�o�������Ă��܂���B" );
					RELEASE_3D(pD3D);
					return FALSE;
				}
			}
		}
	}else{
		if(D3DMain.m_WindowMode){	
		}else{
			RELEASE_3D(pD3DDevice);
			ZeroMemory(&d3dppApp,sizeof(d3dppApp));

	
			d3dppApp.SwapEffect = FULL_FLIP;
	
	
			d3dppApp.BackBufferCount = 1;
			d3dppApp.BackBufferWidth  = WinWidth;
			d3dppApp.BackBufferHeight = WinHeight;

		
			d3dppApp.Windowed = FALSE;							
			d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;


			if(DrawSetting.full_16bit){
				d3dppApp.BackBufferFormat=D3DFMT_UNKNOWN;
				for(i=0;i<D3DCapsStruct.m_DisplayModeNum;i++){
					if( D3DCapsStruct.m_DisplayMode[i].Width  == 800 && D3DCapsStruct.m_DisplayMode[i].Height == 600 ){
						switch( D3DCapsStruct.m_DisplayMode[i].Format ){
							case D3DFMT_R5G6B5:		case D3DFMT_X1R5G5B5:
							case D3DFMT_A1R5G5B5:	case D3DFMT_A4R4G4B4:
							case D3DFMT_X4R4G4B4:
								D3DCapsStruct.m_FullModeNum=i;
								d3dppApp.BackBufferFormat = D3DCapsStruct.m_DisplayMode[i].Format;	
								break;
						}
					}
					if(d3dppApp.BackBufferFormat!=D3DFMT_UNKNOWN) break;
				}
			}else{
				d3dppApp.BackBufferFormat = D3DCapsStruct.m_NowDisplayMode.Format;
			}

			ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
			if( FAILED(ret) ){	
				ret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dppApp,&pD3DDevice);
				if( FAILED(ret) ){	
					switch(ret){
					default:
					case D3DERR_OUTOFVIDEOMEMORY:
						DebugBox( NULL, "Direct3D ���������s���̂ɏ\���ȃf�B�X�v���C ������������܂���B" );
						RELEASE_3D(pD3D);
						return FALSE;
					case D3DERR_INVALIDCALL:
						DebugBox( NULL, "Direct3D�̏������Ɏ��s���܂���[D3DERR_INVALIDCALL]\n���̃O���t�B�b�N�J�[�h�͕K�v�ȋ@�\���T�|�[�g���Ă��Ȃ����A\n���邢��DirectX8�ɑΉ������h���C�o�������Ă��܂���B" );
						RELEASE_3D(pD3D);
						return FALSE;
					case D3DERR_NOTAVAILABLE:
						DebugBox( NULL, "Direct3D�̏������Ɏ��s���܂���[D3DERR_NOTAVAILABLE]\n���̃O���t�B�b�N�J�[�h�͕K�v�ȋ@�\���T�|�[�g���Ă��Ȃ����A\n���邢��DirectX8�ɑΉ������h���C�o�������Ă��܂���B" );
						RELEASE_3D(pD3D);
						return FALSE;
					}
				}
			}
		}
	}



	if( FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&D3DCapsStruct.m_NowDisplayMode)) ){	
		MessageBox(NULL,"�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂����B[�Ȃɂ䂦�H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
		RELEASE_3D(pD3D);
		return FALSE;
	}
	D3DCapsStruct.m_ttCaps.m_R8G8B8		= IsTextureFormatOk( D3DFMT_R8G8B8,   D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_X8R8G8B8	= IsTextureFormatOk( D3DFMT_X8R8G8B8, D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_A8R8G8B8	= IsTextureFormatOk( D3DFMT_A8R8G8B8, D3DCapsStruct.m_NowDisplayMode.Format);

	D3DCapsStruct.m_ttCaps.m_R5G6B5		= IsTextureFormatOk( D3DFMT_R5G6B5,   D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_X1R5G5B5	= IsTextureFormatOk( D3DFMT_X1R5G5B5, D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_A1R5G5B5	= IsTextureFormatOk( D3DFMT_A1R5G5B5, D3DCapsStruct.m_NowDisplayMode.Format);

	D3DCapsStruct.m_ttCaps.m_X4R4G4B4	= IsTextureFormatOk( D3DFMT_X4R4G4B4, D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_A4R4G4B4	= IsTextureFormatOk( D3DFMT_A4R4G4B4, D3DCapsStruct.m_NowDisplayMode.Format);

	D3DCapsStruct.m_ttCaps.m_A8P8		= IsTextureFormatOk( D3DFMT_A8P8,     D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_P8			= IsTextureFormatOk( D3DFMT_P8,       D3DCapsStruct.m_NowDisplayMode.Format);
	D3DCapsStruct.m_ttCaps.m_A8			= IsTextureFormatOk( D3DFMT_A8,       D3DCapsStruct.m_NowDisplayMode.Format);
	
	D3DD_SetBackBuffer( WinWidth, WinHeight );
	pD3DDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_ARGB(0,0,0,0),1.0,0);

	if(0){
		char	buf2[512];

		wsprintf( buf, "���s�f�B�X�v���C���[�h[%s]\n", TxFmtMode[ LIM(d3dppApp.BackBufferFormat,0,D3DFMT_D3DD_MAX-1) ] );
		MBS_CPY(buf2,buf);

		MBS_CAT(buf2,"�g�p�\�e�N�X�`����\n");
		for(i=1;i<D3DFMT_D3DD_MAX-1;i++){
			if( TxFmtMode[i] ){
				j = IsTextureFormatOk( (D3DFORMAT)i, D3DCapsStruct.m_NowDisplayMode.Format);
				if(j){
					wsprintf(buf,"%-15s[%s]\n", TxFmtMode[i], (j==2)?"OK":"OK(NotRenderTarget)" );
					MBS_CAT(buf2,buf);
				}else{
					wsprintf(buf,"%-15s[%s]\n", TxFmtMode[i], "--Not--" );
					MBS_CAT(buf2,buf);
				}
			}
		}

		
		DebugBox(NULL,buf2);



	}

	D3DD_CreateTable();

	return TRUE;
}


int D3DM_TermDevices(void)
{
	D3DD_ResetAll();
	GFree(D3DCapsStruct.m_DisplayMode);

	RELEASE_3D(pD3DDevice);
	RELEASE_3D(pD3D);

	return TRUE;
}



BOOL D3DM_ChangeF2W( void )
{
	int	i;
	if(!D3DMain.m_WindowModeChange) return FALSE;
	D3DMain.m_WindowModeChange = FALSE;

	if( pD3DDevice && pD3D ){
		D3DMain.m_DisplayModeChange=2;
		if( D3DMain.m_FullScreenOnly ){
			SetWindowLong(D3DMain.m_DrawHwnd, GWL_STYLE, FullModeStyle);
			SetMenu(D3DMain.m_DrawHwnd, NULL);
			return FALSE;
		}
		D3DMain.m_WindowMode = !D3DMain.m_WindowMode;

		if( D3DMain.m_WindowMode==TRUE ){

			D3DD_ResetAllRenderTargetTexture();

			
			ZeroMemory(&d3dppApp,sizeof(d3dppApp));

			d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
			d3dppApp.Windowed = D3DMain.m_WindowMode;					


			d3dppApp.SwapEffect = D3DSWAPEFFECT_COPY;				
			d3dppApp.BackBufferFormat = D3DCapsStruct.m_WindowDisplayMode.Format;
			d3dppApp.BackBufferCount = 1;
			d3dppApp.BackBufferWidth  = WinWidth;
			d3dppApp.BackBufferHeight = WinHeight;


			if( FAILED(pD3DDevice->Reset(&d3dppApp)) ){
				D3DMain.m_FlipActive=FALSE;
				
				ZeroMemory(&d3dppApp,sizeof(d3dppApp));
				d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
				d3dppApp.Windowed = D3DMain.m_WindowMode=FALSE;			

				d3dppApp.SwapEffect = FULL_FLIP;				
				d3dppApp.BackBufferFormat = D3DCapsStruct.m_NowDisplayMode.Format;
				d3dppApp.BackBufferCount = 1;
				d3dppApp.BackBufferWidth  = WinWidth;
				d3dppApp.BackBufferHeight = WinHeight;


				pD3DDevice->Reset(&d3dppApp);
				
				return FALSE;
			}

			
			if( FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&D3DCapsStruct.m_NowDisplayMode)) ){	
				MessageBox(NULL,"�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂����B[�Ȃɂ䂦�H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
				return FALSE;
			}

			
			SetWindowLong(D3DMain.m_DrawHwnd, GWL_STYLE, WindowStyle);
			SetMenu(D3DMain.m_DrawHwnd, D3DMain.m_MenuHwnd );	
			RECT rect;
			SetRect( &rect, 0, 0, WinWidth, WinHeight );
			AdjustWindowRect( &rect, WindowStyle, !!D3DMain.m_MenuHwnd );

			int	wx = (GetSystemMetrics(SM_CXFULLSCREEN)-(rect.right-rect.left))/2;
			int	wy = (GetSystemMetrics(SM_CYFULLSCREEN)-(rect.bottom-rect.top))/2;

			wx = WinX;
			wy = WinY;

			SetWindowPos( D3DMain.m_DrawHwnd, HWND_NOTOPMOST, wx, wy, rect.right-rect.left, rect.bottom-rect.top , SWP_SHOWWINDOW|SWP_DRAWFRAME );

		}else{
			D3DMain.m_FlipActive=FALSE;

			
			if( FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&D3DCapsStruct.m_WindowDisplayMode)) ){	
				MessageBox(NULL,"�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂����B[�Ȃɂ䂦�H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
				return FALSE;
			}
			
			D3DD_ResetAllRenderTargetTexture();

			
			ZeroMemory(&d3dppApp,sizeof(d3dppApp));
			d3dppApp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
			d3dppApp.Windowed = D3DMain.m_WindowMode;			

			d3dppApp.SwapEffect = D3DSWAPEFFECT_COPY;

			d3dppApp.BackBufferCount = 1;
			d3dppApp.BackBufferWidth  = WinWidth;
			d3dppApp.BackBufferHeight = WinHeight;
			if(DrawSetting.full_16bit){
				d3dppApp.BackBufferFormat=D3DFMT_UNKNOWN;
				for(i=0;i<D3DCapsStruct.m_DisplayModeNum;i++){
					if( D3DCapsStruct.m_DisplayMode[i].Width  == 800 && D3DCapsStruct.m_DisplayMode[i].Height == 600 ){
						switch( D3DCapsStruct.m_DisplayMode[i].Format ){
							case D3DFMT_R5G6B5:		case D3DFMT_X1R5G5B5:
							case D3DFMT_A1R5G5B5:	case D3DFMT_A4R4G4B4:
							case D3DFMT_X4R4G4B4:
								D3DCapsStruct.m_FullModeNum=i;
								d3dppApp.BackBufferFormat = D3DCapsStruct.m_DisplayMode[i].Format;	
								break;
						}
					}
					if(d3dppApp.BackBufferFormat!=D3DFMT_UNKNOWN) break;
				}
			}else{
				d3dppApp.BackBufferFormat = D3DCapsStruct.m_NowDisplayMode.Format;
			}



	
	
	
	
	

			pD3DDevice->Reset(&d3dppApp);

			SetWindowLong(D3DMain.m_DrawHwnd, GWL_STYLE, FullModeStyle);
			SetMenu(D3DMain.m_DrawHwnd, NULL);	
			SetWindowPos( D3DMain.m_DrawHwnd, HWND_NOTOPMOST, 0, 0, 800,600, SWP_SHOWWINDOW|SWP_DRAWFRAME );
			
			if( FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&D3DCapsStruct.m_NowDisplayMode)) ){	
				MessageBox(NULL,"�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂����B[�Ȃɂ䂦�H]","�v���I�ȃG���[", MB_OK | MB_ICONSTOP);
				return FALSE;
			}
		}
	}
	return TRUE;
}
