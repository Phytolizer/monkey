#pragma once

#include "monkey.h"
#include "monkey/ast.h"
#include "monkey/environment.h"
#include "monkey/object.h"

Object* Eval(Monkey* monkey, Environment* env, Node* node);
