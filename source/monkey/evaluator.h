#pragma once

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/object.h"

Object* Eval(Monkey* monkey, Node* node);
