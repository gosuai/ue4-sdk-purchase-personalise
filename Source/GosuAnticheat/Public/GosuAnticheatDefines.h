// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "CoreMinimal.h"

#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGosuAnticheat, Log, All);

#define VA_FUNC (FString(__FUNCTION__))				 // Current Class Name + Function Name where this is called
#define VA_LINE (FString::FromInt(__LINE__))		 // Current Line Number in the code where this is called
#define VA_FUNC_LINE (VA_FUNC + "(" + VA_LINE + ")") // Current Class and Line Number where this is called!

#define GOSU_ANTICHEAT_VERSION TEXT("v1.0-a1")

#define GOSU_DEFAULT_RECOMMENDATION_SIZE 20