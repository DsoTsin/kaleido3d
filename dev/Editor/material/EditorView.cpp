#include "EditorView.h"

std::shared_ptr<DataModelRegistry>
registerDataModels()
{
auto ret = std::make_shared<DataModelRegistry>();
ret->registerModel<TextSourceDataModel>();
return ret;
}