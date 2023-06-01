#pragma once

#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>
#include <bitset>


#include "trg/klm/modules/klmtrigger/group_helper.h"
#include "trg/klm/modules/klmtrigger/KLMAxis.h"
#include "trg/klm/modules/klmtrigger/KLM_Trig.h"
#include "klm/dataobjects/KLMElementNumbers.h"

namespace Belle2 {
  std::vector<Belle2::group_helper::KLM_trig_linear_fit> klm_trig_linear_fit(std::vector<Belle2::group_helper::KLM_Digit_compact>&
      hits,
      const std::vector<Belle2::group_helper::KLM_geo_fit_t>& geometry);



}