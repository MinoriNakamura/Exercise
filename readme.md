# Exercise
前期提出課題

## プロトタイプ宣言 ##
*グローバル*  
関数  
構造体  

## WinMain ##
メイン関数  
`const TCHAR API_NAME`はアプリケーション名  
window生成  
window生成のパラメータ設定  
メインループ  

## WindowProcedure関数 ##
`LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)`
## DxDeviceの作成関数 ##
`HRESULT BuildDxDevice(HWND hWnd,const TCHAR* filePath)`
## D3dの初期化関数 ##
`HRESULT InitD3d(HWND hWnd,const TCHAR* filePath)`
## PresentParametersの初期化関数 ##
`void InitPresentParameters(HWND hWnd)`
## InitDinputの初期化関数 ##
`HRESULT InitDinput(HWND hWnd)`  
## UpdateKeyStatus関数 ##
`void UpdateKeyStatus()`  
## GetKeyStatus関数 ##
`bool GetKeyStatus(int KeyNumber)`  