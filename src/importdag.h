/**  
 * @file    importdag.h
 * @brief   Import a DAG in other forms
 * @author  Chenyu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */
#ifndef _IMPORTDAG_
#define _IMPORTDAG_
#include <string>
#include <map>
#include "pimconfig.h"
#include "booleandag.h"

BooleanDag *v2booleandag(const std::string &filename);


#endif