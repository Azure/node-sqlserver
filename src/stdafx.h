
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