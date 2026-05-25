#include "progressive/auth_models.hpp"
#include "progressive/sync_models.hpp"
#include "progressive/backup_utils.hpp"
#include "progressive/media_viewer.hpp"
#include <string>

namespace progressive {

std::string canonicalizeJson(const std::string& json) { return json; }
SyncResponse parseSyncResponse(const std::string& json) { SyncResponse sr; return sr; }
std::string getMediaTypeName(MediaType) { return "unknown"; }

} // namespace progressive
