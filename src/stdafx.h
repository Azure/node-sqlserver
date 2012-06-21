//---------------------------------------------------------------------------------------------------------------------------------
// File: stdafx.h
// Contents: Precompiled header
// 
// Copyright Microsoft Corporation and contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// You may obtain a copy of the License at:
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//---------------------------------------------------------------------------------------------------------------------------------

#pragma once

#include <v8.h>
#include <uv.h>
#include <node.h>
#include <node_buffer.h>

#include <sqlucode.h>

#include <vector>
#include <queue>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>

#include "Utility.h"
#include "OdbcException.h"
#include "OdbcHandle.h"

#define ErrorIf(x) if (x) goto Error;
