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
#pragma once
#include "../../Graphics/Geometry.h"
#include "../../Graphics/Text.h"
#include "MapleFrame.h"
#include "Tooltip.h"

namespace jrc
{
class SkillTooltip : public Tooltip
{
public:
    SkillTooltip();

    void draw(Point<std::int16_t> position) const override;

    void set_skill(std::int32_t id,
                   std::int32_t level,
                   std::int32_t masterlevel,
                   std::int64_t expiration);

private:
    std::int32_t skill_id;
    std::int16_t height;
    std::int16_t icon_offset;
    std::int16_t level_offset;
    Texture icon;
    Texture required_icon;

    Text name;
    Text desc;
    Text leveldesc;
    MapleFrame frame;
    ColorLine line;
    Texture base;
    Texture cover;
};
} // namespace jrc
