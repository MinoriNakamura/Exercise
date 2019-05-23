/*
	2019/05/22 created by ohura

	参考 先輩のサンプルコード
		 授業で写した「初めての3Dゲーム開発」のコード
	   	 昨年小島先生の授業でやったコード
		 Google
*/

#include <windows.h>
#include <d3dx9.h>
#include <dinput.h>
#include <tchar.h>

//Direct3D
LPDIRECT3D9 pDirect3D;
//Direct3Dデバイス
LPDIRECT3DDEVICE9 pDevice;
//ウィンドウ設定
D3DPRESENT_PARAMETERS D3dPresentParameters;
//DirectInputのキーボードデバイス
LPDIRECTINPUTDEVICE8 pDxIKeyDevice;
//DirectInputのインターフェース
LPDIRECTINPUT8 pDinput;
//テクスチャ
LPDIRECT3DTEXTURE9 pTexture;

//XYZRHW 頂点座標(除算あり)
//DiFFUSE 拡散色成分
//TEX1 テクスチャ座標
const int D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
//入力キーの最大数
static const int MAX_KEY_NUMBER = 256;
//入力キー判定のマスク値
const int MASK_NUM = 0x80;
BYTE KeyState[MAX_KEY_NUMBER];

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
//Direct3D初期化関数
HRESULT InitD3d(HWND,const TCHAR*);
//プレゼントパラメータ初期化関数
void InitPresentParameters(HWND);
//Dinput初期化関数
HRESULT InitDinput(HWND);
//キーステータス更新関数
void UpdateKeyStatus();
//キー入力関数
bool GetKeyStatus(int);
//デバイス作成
HRESULT BuildDxDevice(HWND hWnd, const TCHAR* filePath);

//ウィンドウハンドル
static HWND hWnd;

//CUSTOMVERTEX(頂点情報などの構造体)
struct CUSTOMVERTEX
{
	//x,y,zは座標位置 rhwは除算数(スケール変換)
	FLOAT x, y, z, rhw;
	//色情報
	DWORD color;
	//テクスチャの座標
	FLOAT tu, tv;
};

//アプリケーションの名前
const TCHAR API_NAME[] = _T("Exercise");

INT WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdline,int nCmdShow) {

	//システム時間の取得及び宣言と代入
	DWORD SyncPrev = timeGetTime();
	DWORD SyncCurr;
	//メッセージ
	MSG msg;

	CUSTOMVERTEX customvertex[4]{
	{10,10,0,1,0xFFFFFF,0,0},
	{200,10,0,1,0xFFFFFF,1,0},
	{200,200,0,1,0xFFFFFF,1,1},
	{10,200,0,1,0xFFFFFF,0,1}
	};

	//ウィンドウクラス
	WNDCLASS Wndclass;
	Wndclass.style = CS_HREDRAW | CS_VREDRAW; //ウィンドウスタイル
	Wndclass.lpfnWndProc = WndProc; //ウィンドウプロシージャ
	Wndclass.cbClsExtra = 0; //メモリ確保
	Wndclass.cbWndExtra = 0; //メモリ確保
	Wndclass.hInstance = hInstance;	//ハンドルインスタンス
	Wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION); //アイコン
	Wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);	//カーソル
	Wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //背景のブラシ,色
	Wndclass.lpszMenuName = NULL; //メニュー画面の名前
	Wndclass.lpszClassName = API_NAME; //アプリケーションの名前

	//ウィンドウクラスの登録
	RegisterClass(&Wndclass);

	//ウィンドウハンドルにcreatewindow関数で作った情報を代入
	HWND hWnd = CreateWindow(
		API_NAME, //クラスの名前
		API_NAME, //アプリケーションのタイトル
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, //ウィンドウのスタイル
		CW_USEDEFAULT, //Xの位置
		CW_USEDEFAULT, //Yの位置
		640, //幅
		480, //高さ
		NULL, //親ウィンドウのハンドル
		NULL, //メニューのハンドル
		hInstance, //インスタンスハンドル
		NULL //メッセージに渡されるパラメータ
	);

	//ウィンドウ情報をみて更新
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	//テスト用のファイルでDirect3Dを作成
	//_TはマルチバイトとUnicodeの変換処理をしてくれるマクロ
	BuildDxDevice(hWnd, _T("Blank.jpg"));

	//画像ファイルからテクスチャを読み込み、pTextureにその情報を入れる
	D3DXCreateTextureFromFile(
		pDevice,
		_T("nigaoe.png"),
		&pTexture);

	//メインループ//

	//時間の有効数字
	timeBeginPeriod(1);
	//メッセージの中身の消去
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {
		//1ms止まる
		Sleep(1);
		//メッセージ覗く
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			//メッセージを翻訳する
			TranslateMessage(&msg);
			//メッセージを送る
			DispatchMessage(&msg);
		}
		else {
			//システム時間をSyncCurrに入れる
			SyncCurr = timeGetTime();
			//SyncCurrとSyncPrevの差が1000分の60秒になったとき(単位がms[1sは1000ms]かつ60hz[60fps]で動かすには1秒間に60回絵の更新が入るから)
			if (SyncCurr - SyncPrev >= 1000 / 60) {
				//ウィンドウクリア
				pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0, 0);
				//テクスチャ貼り付け開始
				pDevice->BeginScene();

				//キー状態の確認と更新
				UpdateKeyStatus();

				//操作系のやつ
				//終了時の処理
				if (GetKeyStatus(DIK_RETURN)) {
					break;
				}
				//それぞれのキーに対応する方向に絵を動かす処理
				//上キーを押されたとき
				if (GetKeyStatus(DIK_UP)) {
					customvertex[0].y -= 5.0f;
					customvertex[1].y -= 5.0f;
					customvertex[2].y -= 5.0f;
					customvertex[3].y -= 5.0f;
				}
				//下キーを押されたとき
				if (GetKeyStatus(DIK_DOWN)) {
					customvertex[0].y += 5.0f;
					customvertex[1].y += 5.0f;
					customvertex[2].y += 5.0f;
					customvertex[3].y += 5.0f;
				}
				//左キーを押されたとき
				if (GetKeyStatus(DIK_LEFT)) {
					customvertex[0].x -= 5.0f;
					customvertex[1].x -= 5.0f;
					customvertex[2].x -= 5.0f;
					customvertex[3].x -= 5.0f;
				}
				//右キーを押されたとき
				if (GetKeyStatus(DIK_RIGHT)) {
					customvertex[0].x += 5.0f;
					customvertex[1].x += 5.0f;
					customvertex[2].x += 5.0f;
					customvertex[3].x += 5.0f;
				}

				//テクスチャ貼り付け
				pDevice->SetTexture(0, pTexture);
				//大きさなどの設定された情報で書き始める
				pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, customvertex, sizeof(CUSTOMVERTEX));
				//テクスチャ貼り付け終わり
				pDevice->EndScene();
				//絵を表示する
				pDevice->Present(0, 0, 0, 0);
				//さっきのSyncCurrをSyncPrevに代入する
				SyncPrev = SyncCurr;
			}
			
		}
	}
	timeEndPeriod(1);

	//リリース(解放) 全部初期化とは逆の順番で解放する
	if (pDxIKeyDevice)
	{
		pDxIKeyDevice->Unacquire();
	}
	pDxIKeyDevice->Release();
	pDxIKeyDevice = nullptr;
	pDinput->Release();
	pDinput = nullptr;
	pDevice->Release();
	pDevice = nullptr;
	pDirect3D->Release();
	pDirect3D = nullptr;
	pTexture->Release();
	pTexture = nullptr;

	return (int)msg.wParam;
}

//ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	//メッセージ別の処理
	switch (msg)
	{
	//ウィンドウが破壊されたときのメッセージ
	case WM_DESTROY:
		//閉じるメッセージを送る
		PostQuitMessage(0);
		return 0;
	//ユーザーがメニューかALTキーと何かを押したとき
	case WM_SYSKEYDOWN:
		switch (wp) 
		{
		case VK_RETURN:
			return 0;
		case VK_F4:
			//閉じるメッセ―ジ
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		default:
			return 0;
		}
		return 0;
	}
	//DefaultWindowProcedureを返す(デフォルト)
	return DefWindowProc(hWnd, msg, wp, lp);
}

//Direct3Dの作成
HRESULT BuildDxDevice(HWND hWnd, const TCHAR* filePath) {
	//D3dの初期化に失敗したとき
	if (FAILED(InitD3d(hWnd, filePath))) {
		return E_FAIL;
	}

	//Dinputの初期化に失敗したとき
	if (FAILED(InitDinput(hWnd)))
	{
		MessageBox(0, _T("DirectInputDeviceの作成に失敗しました"), NULL, MB_OK);
		return E_FAIL;
	}

	//D3Dのポインタ変数にDirect3DCreate9関数(Direct3Dを作る関数)で作成したものを代入
	pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

	//D3Dのポインタ変数がNULLだった時
	if (pDirect3D == NULL) {
		//D3Dの作成に失敗した時の処理
		MessageBox(0, _T("Direct3Dの作成に失敗しました"), NULL, MB_OK);
		return E_FAIL;
	}
	//レンダーの設定(透過処理など)
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	//頂点情報をセット
	pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	return S_OK;
}

//Direct3Dの初期化
HRESULT InitD3d(HWND hWnd,const TCHAR* filePath) {
	//Direct3Dの作成
	if (NULL == (pDirect3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(0,_T("Direct3Dの作成に失敗しました"),NULL,MB_OK);
		return E_FAIL;
	}
	//PresentParametersの初期化
	InitPresentParameters(hWnd);

	//デバイスオブジェクトの作成
	if (FAILED(pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_MIXED_VERTEXPROCESSING,
		&D3dPresentParameters, &pDevice)))
	{
		MessageBox(0, _T("HALモードでDIRECT3Dデバイスを作成できませんでした"), NULL, MB_OK);
		if (FAILED(pDirect3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_REF,hWnd,
			D3DCREATE_MIXED_VERTEXPROCESSING,
			&D3dPresentParameters, &pDevice))) {
			MessageBox(0, _T("DIRECT3Dデバイスの作成に失敗しました"),NULL, MB_OK);
			return E_FAIL;
		}
	}

	//テクスチャオブジェクトの作成
	if (FAILED(D3DXCreateTextureFromFileEx(pDevice, filePath, 100, 100, 0, 0, D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,D3DX_FILTER_NONE,D3DX_DEFAULT,
		0xff000000, NULL, NULL, &pTexture)))
	{
		MessageBox(0,_T("テクスチャオブジェクトの作成に失敗しました"), NULL, MB_OK);
		return E_FAIL;
	}
	//ClearTexture();

	return S_OK;
}

void InitPresentParameters(HWND hWnd) {
	//PresentParametersの中身を消去
	ZeroMemory(&D3dPresentParameters, sizeof(D3DPRESENT_PARAMETERS));
	D3dPresentParameters.BackBufferWidth = 0; //バックバッファの横幅
	D3dPresentParameters.BackBufferHeight = 0; // バックバッファの高さ
	D3dPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN; //バックバッファのフォーマット
	D3dPresentParameters.BackBufferCount = 1; //バックバッファの数
	D3dPresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE; //マルチサンプルのタイプ
	D3dPresentParameters.MultiSampleQuality = 0; //マルチサンプルのクオリティ
	D3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD; //バッファーの入れ替え方法(バックバッファとフロントバッファ)
	D3dPresentParameters.hDeviceWindow = hWnd; //表示するウィンドウのハンドル
	D3dPresentParameters.Windowed = TRUE; //ウィンドウモード
	D3dPresentParameters.EnableAutoDepthStencil = FALSE; //深度バッファを管理するかどうかの設定
	D3dPresentParameters.Flags = 0; //バックバッファをロックするかどうか
	D3dPresentParameters.FullScreen_RefreshRateInHz = 0; //ディスプレイのリフレッシュレート
	D3dPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //スワップのバックバッファを表示できる最大速度
}

//Dinput初期化
HRESULT InitDinput(HWND hWnd) {
	HRESULT hr;
	// DirectInputオブジェクトの作成
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID * *)& pDinput, NULL))){
		return hr;
	}

	//DirectInputデバイスオブジェクトの作成
	if (FAILED(hr = pDinput->CreateDevice(GUID_SysKeyboard,
		&pDxIKeyDevice, NULL))){
		return hr;
	}

	//デバイスのフォーマットを設定(今回はキーボード)
	if (FAILED(hr = pDxIKeyDevice->SetDataFormat(&c_dfDIKeyboard))) {
		return hr;
	}

	//協調レベルの設定
	if (FAILED(hr = pDxIKeyDevice->SetCooperativeLevel(
		hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND))) {
		return hr;
	}

	//キー情報更新
	pDxIKeyDevice->Acquire();
	return S_OK;
}

//キー状態更新
void UpdateKeyStatus() {
	//デバイスのポインタ変数の情報を更新する
	HRESULT hr = pDxIKeyDevice->Acquire();
	if ((hr == DI_OK) || (hr == S_FALSE)) {
		//デバイスのポインタ変数にデバイス情報を入れる
		pDxIKeyDevice->GetDeviceState(sizeof(KeyState), &KeyState);
	}
}

//キー入力
bool GetKeyStatus(int KeyNumber) {
	//キーの入力状態を確認する
	//キーナンバーと0x80をビット演算する
	if (KeyState[KeyNumber] & 0x80){
		return true;
	}
	return false;
}
