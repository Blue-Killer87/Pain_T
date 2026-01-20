#ifndef TOOL_FACTORY_H
#define TOOL_FACTORY_H

#include "tool.h"
#include "tool_id.h"
#include "theme.h"

Tool* tool_create(ToolID id, Theme* theme);

#endif
