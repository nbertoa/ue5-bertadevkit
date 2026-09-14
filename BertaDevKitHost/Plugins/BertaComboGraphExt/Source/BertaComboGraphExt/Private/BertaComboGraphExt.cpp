#include "BertaComboGraphExt.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBertaComboGraphExt);

// Runtime integrations are opt-in components/tasks; the module registers no global gameplay state.
IMPLEMENT_MODULE(FDefaultModuleImpl, BertaComboGraphExt)
