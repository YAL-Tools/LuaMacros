#include "stdafx.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <stdio.h>
enum class Result {
	Trouble = -1,
	IsNotMuted,
	IsMuted,
};
enum class Action {
	GetMuted,
	SetMute,
	SetUnmute,
	ToggleMute,
};
static Result impl(Action action) {
	// https://stackoverflow.com/a/3046715/5578773
	HRESULT hr;

	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
	if (hr != S_OK) return Result::Trouble;

	IMMDevice* defaultDevice = NULL;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	if (defaultDevice == NULL) return Result::Trouble;

	IAudioEndpointVolume* endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;

	BOOL muted;
	hr = endpointVolume->GetMute(&muted);
	if (action != Action::GetMuted) {
		switch (action) {
			case Action::SetMute: muted = true; break;
			case Action::SetUnmute: muted = false; break;
			case Action::ToggleMute: muted = !muted; break;
		}
		hr = endpointVolume->SetMute(muted, NULL);
		if (hr != S_OK) {
			printf("SetMute failed, HRESULT %x\n", hr);
		}
	}
	endpointVolume->Release();
	return muted ? Result::IsMuted : Result::IsNotMuted;
}
static void pushResult(lua_State* q, Result r) {
	if (r == Result::Trouble) {
		lua_pushnil(q);
	} else {
		lua_pushboolean(q, r == Result::IsMuted);
	}
}
static int get(lua_State* q) {
	auto r = impl(Action::GetMuted);
	pushResult(q, r);
	return 1;
}
static int set(lua_State* q) {
	auto z = lua_toboolean(q, -1);
	auto r = impl(z ? Action::SetMute : Action::SetUnmute);
	pushResult(q, r);
	return 1;
}
static int toggle(lua_State* q) {
	auto z = lua_toboolean(q, -1);
	auto r = impl(Action::ToggleMute);
	pushResult(q, r);
	return 1;
}
void init_micmute_lualib(lua_State* q) {
	lua_newtable(q);
	luaM_rawset_str_cfunc(q, "get", get);
	luaM_rawset_str_cfunc(q, "set", set);
	luaM_rawset_str_cfunc(q, "toggle", toggle);
	lua_setglobal(q, "micmute");
}