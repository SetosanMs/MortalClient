//////////////////////////////////////////////////////////////////////////////
// This file is part of the LibreMaple MMORPG client                        //
// Copyright © 2015-2016 Daniel Allendorf, 2018-2019 LibreMaple Team        //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <https://www.gnu.org/licenses/>.   //
//////////////////////////////////////////////////////////////////////////////
#include "SkillBullet.h"

#include "../../Data/BulletData.h"
#include "../../Util/Misc.h"

namespace jrc
{
Animation RegularBullet::get(const Char&, std::int32_t bulletid) const
{
    return BulletData::get(bulletid).get_animation();
}

SingleBullet::SingleBullet(nl::node src)
{
    ball = src["ball"];
}

Animation SingleBullet::get(const Char&, std::int32_t) const
{
    return ball.animation;
}

BySkillLevelBullet::BySkillLevelBullet(nl::node src, std::int32_t id)
{
    skillid = id;

    for (auto sub : src["level"]) {
        auto level = string_conversion::or_zero<std::int32_t>(sub.name());
        bullets[level] = sub["ball"];
    }
}

Animation BySkillLevelBullet::get(const Char& user, std::int32_t) const
{
    std::int32_t level = user.get_skill_level(skillid);
    auto iter = bullets.find(level);
    if (iter != bullets.end()) {
        return iter->second.animation;
    } else {
        return {};
    }
}
} // namespace jrc
