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
#include "../../Net/Login.h"
#include "../UIElement.h"

namespace jrc
{
class UIWorldSelect : public UIElement
{
public:
    static constexpr const Type TYPE = WORLD_SELECT;
    static constexpr const bool FOCUSED = false;
    static constexpr const bool TOGGLED = false;

    UIWorldSelect(std::vector<World>&& worlds_);

    void draw(float alpha) const override;

    std::uint8_t get_world_id() const;
    std::uint8_t get_channel_id() const;

protected:
    Button::State button_pressed(std::uint16_t buttonid) override;

private:
    enum Buttons { BT_ENTERWORLD = 0, BT_WORLD0 = 1, BT_CHANNEL0 = 17 };

    std::vector<World> worlds;
    std::uint8_t world_id;
    std::uint8_t channel_id;
};
} // namespace jrc
