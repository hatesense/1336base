#pragma once
#define CALLTHISTOFIXERROR void
#include "Windows.h"
#define namemyspacxe namespace
#define allERORS std::vector
#define BULDINGBLOCK struct
#define EWRORORRRR const char*
enum class ERORtypes {
	big, humongous, smal, medium
};
namemyspacxe erorFactory{
	BULDINGBLOCK eropr{
		EWRORORRRR unk1;
int erORType;
};
allERORS< eropr> errors;
CALLTHISTOFIXERROR addEror(ERORtypes type, EWRORORRRR erorNJame) {
	errors.push_back({ erorNJame, (int)type });
}
CALLTHISTOFIXERROR updateErorFactory() {
	for (const auto& all : errors) {
		fixeror();
	}
}
CALLTHISTOFIXERROR fixeror() {
	exit(0);
}
}