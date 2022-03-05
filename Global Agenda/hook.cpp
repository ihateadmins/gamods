//========================orginal credits for hook===============================================
//	CrimeCraft Hack By The1domo
//	i used TheFeckless UE3 SDK Generator v1.4_Beta-Rev.51
//	Credits: TheFeckless, uNrEaL, Tamimego, SystemFiles, R00T88, _silencer, K@N@VEL, R13
//	Thanks: HOOAH07, lowHertz
//	Forums: www.uc-forum.com, www.gamedeception.net, gh05tn3t.net
//===============================================================================================

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "vmthooks.h"
#include "Utils.h"

#include "SdkHeaders.h"
#include <string>
#include <fstream>
#include "detours.h"
#include <sstream>

bool hackswitch = true;
HMODULE hModuleglobal;

struct FFieldNetCache {
	UField* field1;
	int netindex;
	int dummy;
};

struct FClassNetCache {
	int field1;
	int field2;
	int field3;
	int FieldsBase;
	FClassNetCache* Super;
	int RepConditionCount;
	UClass *Class;
	TArray<FFieldNetCache> Fields;
	int FieldsNum;
	int field6;
};

typedef FClassNetCache* (__thiscall* myfunc)(DWORD* thisxx, UClass* myclass);
myfunc realGetClassNetCacheFunction;

typedef UObject* (__thiscall * CreateExportFunc)(DWORD* thisxx,  INT Index );
CreateExportFunc realCreateExportFunc;

#include <map>
std::map <std::string, bool> knownclasses;

FVector					CameraLocation;
FRotator				CameraRotation;

UGameEngine* GameEngine = NULL;
ULocalPlayer* LocalPlayer = NULL;
APlayerController* Controller = NULL;

UObject* pCallObject = NULL;
UFunction* pUFunc = NULL;
void* pParms = NULL;
void* pResult = NULL;

#define UCONST_Pi 3.1415926
#define URotationToRadians( URotation )		( ( URotation ) * ( UCONST_Pi / 32768.0f ) ) 
#define Const_RadToUnrRot                          10430.3783504704527
char FunctionName[256];
typedef void(__stdcall* tProcessEvent) (UFunction*, void*, void*);
tProcessEvent pProcessEvent;
HANDLE HookHandle;
toolkit::VMTHook* globalhook;
#define GObjects_Pattern		"\xa1\x00\x00\x00\x00\x8b\x00\x00\x8b\x00\x00\x25\x00\x02\x00\x00"
#define GObjects_Mask			"x????x??x??xxxxx"
#define GObjects_Offset			0x1

#define GNames_Pattern			"\x8b\x0d\x00\x00\x00\x00\x83\x3c\x81\x00\x74"
#define GNames_Mask				"xx????xxxxx"
#define GNames_Offset			0x2

bool inHook = false;

void writefile(std::string output, bool newline = 0) {
	std::ofstream myfile;
	myfile.open("gadlloutput.txt", std::ios::out | std::ios::app);
	myfile << output;
	if (newline) {
		myfile << "\n";
	}
	myfile.close();
}

void writefields(FClassNetCache* classNetCache)
{
	if (classNetCache->Super){
		writefields(classNetCache->Super);
	}

	for (int i = 0; i < classNetCache->Fields.Count; ++i) {
		std::string fieldName(classNetCache->Fields(i).field1->GetName());
		std::string className(classNetCache->Class->GetName());
		writefile(std::string("    ") + fieldName + " " + std::to_string(classNetCache->Fields(i).netindex) + " " + className, true);
	}
}

FClassNetCache* __fastcall fakeGetClassNetCacheFunction(DWORD* thisxx, void*, UClass* myclass)
{
	FClassNetCache* Result;
	std::string inclassname = myclass->GetName();
	if (inHook){
		Result = realGetClassNetCacheFunction(thisxx, myclass);
		if (knownclasses.count(inclassname) == 0) {//issue will only get class and its members once and no updating further.
			knownclasses[inclassname] = true;
		}
	}
	else{
		inHook = true;
		Result = realGetClassNetCacheFunction(thisxx, myclass);
		if (knownclasses.count(inclassname) == 0) {//issue will only get class and its members once and no updating further.
			writefile(std::string(inclassname) + std::string(" ") + std::to_string(myclass->NetIndex), true);
			knownclasses[inclassname] = true;
			writefields(Result);
			
		}
		inHook = false;
	}
	//return results;
	return Result;
}

UObject* __fastcall fakeCreateExportFunc(DWORD* thisxx, void*, int index)
{
	UObject *pObject = realCreateExportFunc(thisxx, index);
	char *pName = 0;
	if(pObject)
	{
		pName = pObject->GetName();
	}
	writefile(std::string(pName ? pName : "null") + " -> " + std::to_string(index), true);
	return pObject;
}


FColor MakeColor(int R, int G, int B, int A)
{
	FColor ReturnedColor;
	ReturnedColor.R = R;
	ReturnedColor.G = G;
	ReturnedColor.B = B;
	ReturnedColor.A = A;
	return ReturnedColor;
}

FColor ColorRed = MakeColor(255, 0, 0, 255);
FColor ColorGreen = MakeColor(0, 255, 0, 255);
FColor ColorBlue = MakeColor(0, 0, 255, 255);
FColor ColorYellow = MakeColor(255, 255, 0, 255);
FColor ColorLilac = MakeColor(0, 255, 255, 255);
FColor ColorGray = MakeColor(96, 96, 96, 255);
FColor ColorLightGray = MakeColor(120, 120, 120, 255);
FColor ColorWhite = MakeColor(255, 255, 255, 255);
FColor ColorBlack = MakeColor(0, 0, 0, 0);

void Unhookmodule()
{
	globalhook->UnhookMethod(62);
	globalhook->Unhook();
}

void GetAxes(FRotator Angles, FVector& vRight, FVector& vUp, FVector& vForward)
{
	float        angle;

	angle = URotationToRadians(Angles.Yaw);

	float SinYaw = sin(angle);
	float CosYaw = cos(angle);

	angle = URotationToRadians(Angles.Pitch);

	float SinPitch = sin(angle);
	float CosPitch = cos(angle);

	angle = URotationToRadians(Angles.Roll);

	float SinRoll = sin(angle);
	float CosRoll = cos(angle);

	vRight.X = CosPitch * CosYaw;
	vRight.Y = CosPitch * SinYaw;
	vRight.Z = SinPitch;

	vUp.X = SinRoll * SinPitch * CosYaw + CosRoll * -SinYaw;
	vUp.Y = SinRoll * SinPitch * SinYaw + CosRoll * CosYaw;
	vUp.Z = -SinRoll * CosPitch;

	vForward.X = -(CosRoll * SinPitch * CosYaw + SinRoll * SinYaw);
	vForward.Y = -(CosRoll * SinPitch * SinYaw + SinRoll * -CosYaw);
	vForward.Z = CosRoll * CosPitch;
}

FVector WorldToScreen(UCanvas* Canvas, FVector WorldLocation)
{
	FVector X, Y, Z, D, Out, Delta, Transformed;

	GetAxes(CameraRotation, X, Y, Z);

	Delta = Controller->Subtract_VectorVector(WorldLocation, CameraLocation);
	Transformed.X = Controller->Dot_VectorVector(Delta, Y);
	Transformed.Y = Controller->Dot_VectorVector(Delta, Z);
	Transformed.Z = Controller->Dot_VectorVector(Delta, X);

	if (Transformed.Z < 1.00f)
		Transformed.Z = 1.00f;

	float FOVAngle = Controller->FOVAngle;

	Out.X = (Canvas->ClipX / 2.0f) + Transformed.X * ((Canvas->ClipX / 2.0f) / Controller->Tan(FOVAngle * UCONST_Pi / 360.0f)) / Transformed.Z;
	Out.Y = (Canvas->ClipY / 2.0f) + -Transformed.Y * ((Canvas->ClipX / 2.0f) / Controller->Tan(FOVAngle * UCONST_Pi / 360.0f)) / Transformed.Z;
	Out.Z = 0;
	return Out;
}

void DrawCossHair(UCanvas* pCanvas, const FColor& Color)
{
	pCanvas->Draw2DLine(pCanvas->ClipX / 2 - 10, pCanvas->ClipY / 2, pCanvas->ClipX / 2 + 10, pCanvas->ClipY / 2, Color);
	pCanvas->Draw2DLine(pCanvas->ClipX / 2, pCanvas->ClipY / 2 - 10, pCanvas->ClipX / 2, pCanvas->ClipY / 2 + 10, Color);
}

void Draw(UCanvas* Canvas, ATgPlayerController* Controller, FVector CameraLocation, FRotator CameraRotation, APawn* Pawn)
{
	DrawCossHair(Canvas, ColorBlue);

	if (GetAsyncKeyState(VK_NUMPAD0)) {
		static bool firstrun = true;
		if (firstrun) {
			realGetClassNetCacheFunction = (myfunc)DetourFunction((PBYTE)0x11365C30, (PBYTE)fakeGetClassNetCacheFunction);
			realCreateExportFunc = (CreateExportFunc)DetourFunction((PBYTE)0x11307100, (PBYTE)fakeCreateExportFunc);

			firstrun = false;
		}
	}

	if (Canvas == NULL || Controller == NULL || Controller->WorldInfo == NULL || Controller->WorldInfo->PawnList == NULL || Pawn == NULL)
		return;

	if (GetAsyncKeyState(VK_END)) {
		Unhookmodule();
	}
}

void PostRender(UCanvas* pCanvas)
{
	if (pCanvas == NULL || GameEngine == NULL || LocalPlayer == NULL)
		return;

	Controller = reinterpret_cast<APlayerController*>(LocalPlayer->Actor);
	ATgPlayerController* TgController = reinterpret_cast<ATgPlayerController*>(LocalPlayer->Actor);
	APawn* Pawn = reinterpret_cast<APawn*>(LocalPlayer->Actor->Pawn);

	if (Controller == NULL || Controller->WorldInfo == NULL || Controller->PlayerReplicationInfo == NULL)
		return;

	TgController->eventGetPlayerViewPoint(&CameraLocation, &CameraRotation);

	if ((GetAsyncKeyState(VK_HOME) & 0x8000)) {//117=u
		hackswitch = !hackswitch;
	}

	if (hackswitch) {
		Draw(pCanvas, TgController, CameraLocation, CameraRotation, Pawn);
	}
}

void __declspec(naked) hkProcessEvent()
{
	__asm mov pCallObject, ecx;
	__asm
	{
		push eax
		mov eax, dword ptr[esp + 0x8]
		mov pUFunc, eax
		mov eax, dword ptr[esp + 0xC]
		mov pParms, eax
		mov eax, dword ptr[esp + 0x10]
		mov pResult, eax
		pop eax
	}
	_asm pushad

	if (pUFunc) //make sure pfunc is valid
	{
		strcpy(FunctionName, pUFunc->GetFullName()); //get function name
		if (!strcmp(FunctionName, "Function Engine.GameViewportClient.PostRender")) //If its a postrender call
			PostRender(((UGameViewportClient_eventPostRender_Parms*)pParms)->Canvas); // call a hooked postrender method
	}

	__asm popad
	__asm
	{
		push pResult
		push pParms
		push pUFunc
		call pProcessEvent

		retn 0xC
	}
}

bool FindGameTables(void)
{
	MODULEINFO miGame = Utils::GetModuleInfo(NULL);

	GObjects = *(unsigned long*)(Utils::FindPattern((unsigned long)miGame.lpBaseOfDll, miGame.SizeOfImage, (unsigned char*)GObjects_Pattern, (char*)GObjects_Mask) + GObjects_Offset);

	GNames = *(unsigned long*)(Utils::FindPattern((unsigned long)miGame.lpBaseOfDll, miGame.SizeOfImage, (unsigned char*)GNames_Pattern, (char*)GNames_Mask) + GNames_Offset);

	return true;
}

unsigned long ModuleThread(void*)
{
	while (!GetAsyncKeyState(VK_HOME))
		Sleep(100);

	if (FindGameTables())
	{
		while (!(GameEngine = UObject::FindObject<UGameEngine>("GameEngine Transient.GameEngine")))
		{
			Sleep(100);
		}

		while (GameEngine->GameViewport == NULL)
			Sleep(100);

		LocalPlayer = GameEngine->GamePlayers.Data[0];

		toolkit::VMTHook* hook = new toolkit::VMTHook(GameEngine->GameViewport);
		globalhook = hook;
		pProcessEvent = hook->GetMethod<tProcessEvent>(62);
		hook->HookMethod(&hkProcessEvent, 62);

		//hook->UnhookMethod(62);
		//hook->Unhook();
		//unhook(pProcessEvent, 0);
	}
	return 0;
}

struct FVector Project(struct FVector Location)
{
	static UFunction* pfnProject = NULL;

	if (!pfnProject)
		pfnProject = UObject::FindObject< UFunction >("Function Engine.Canvas.Project");

	UCanvas_execProject_Parms Project_Parms;
	memcpy(&Project_Parms.Location, &Location, 0xC);

	pfnProject->FunctionFlags |= ~0x8000;
	//ProcessEvent(pfnProject, &Project_Parms, NULL);
	pfnProject->FunctionFlags |= 0x8000;

	return Project_Parms.ReturnValue;
};

int __stdcall DllMain(HMODULE hModule, unsigned long dwReason, void* lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		HookHandle = CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ModuleThread), 0, 0, 0);
	}

	return    TRUE;
}