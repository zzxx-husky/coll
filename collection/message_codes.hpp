#pragma once

#include "zaf/zaf.hpp"

namespace coll {
namespace codes {

const inline zaf::Code Data{1};
const inline zaf::Code Data2{2};
const inline zaf::Code Data3{3};
const inline zaf::Code DataWithQuota{9};
const inline zaf::Code Termination{10};
const inline zaf::Code Downstream{11};
const inline zaf::Code Upstream{12};
const inline zaf::Code Quota{13};
const inline zaf::Code Clear{14};

} // namespace codes
} // namespace coll
