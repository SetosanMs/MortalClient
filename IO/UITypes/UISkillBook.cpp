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
#include "UISkillBook.h"

#include "../../Character/SkillId.h"
#include "../../Data/JobData.h"
#include "../../Data/SkillData.h"
#include "../../Gameplay/Stage.h"
#include "../../IO/UI.h"
#include "../../Net/Packets/PlayerPackets.h"
#include "../../Util/Misc.h"
#include "../Components/MapleButton.h"
#include "../Components/TwoSpriteButton.h"
#include "nlnx/nx.hpp"

namespace jrc
{
constexpr Point<std::int16_t> UISkillbook::SKILL_OFFSET;
constexpr Point<std::int16_t> UISkillbook::ICON_OFFSET;
constexpr Point<std::int16_t> UISkillbook::LINE_OFFSET;

SkillIcon::SkillIcon(std::int32_t i, std::int32_t lv) : id{i}
{
    const SkillData& data = SkillData::get(id);

    normal = data.get_icon(SkillData::NORMAL);
    mouse_over = data.get_icon(SkillData::MOUSEOVER);
    disabled = data.get_icon(SkillData::DISABLED);

    std::string name_str{data.get_name()};

    name = {Text::A11L, Text::LEFT, Text::DARKGREY, std::string{name_str}};
    level = {Text::A11L, Text::LEFT, Text::DARKGREY, std::to_string(lv)};
    state = NORMAL;

    constexpr const std::uint16_t MAX_NAME_WIDTH = 96;
    std::size_t overhang = 3;
    while (name.width() > MAX_NAME_WIDTH) {
        name_str.replace(name_str.end() - overhang, name_str.end(), "...", 3);
        ++overhang;

        name.change_text(std::string{name_str});
    }
}

std::int32_t SkillIcon::get_action_id() const noexcept
{
    return -id;
}

void SkillIcon::draw(const DrawArgument& args) const
{
    switch (state) {
    case NORMAL:
        normal.draw(args);
        break;
    case DISABLED:
        disabled.draw(args);
        break;
    case MOUSE_OVER:
        mouse_over.draw(args);
        break;
    }

    name.draw(args + Point<std::int16_t>{38, -34});
    level.draw(args + Point<std::int16_t>{38, -16});
}

Cursor::State SkillIcon::send_cursor(Point<std::int16_t> cursor_pos,
                                     bool clicked)
{
    static constexpr const Rectangle<std::int16_t> bounds{0, 32, 0, 32};
    bool in_range = bounds.contains(cursor_pos);

    switch (state) {
    case NORMAL:
    case DISABLED:
        if (in_range) {
            if (clicked) {
                state = MOUSE_OVER;
                return Cursor::GRABBING;
            } else {
                state = MOUSE_OVER;
                return Cursor::CAN_GRAB;
            }
        } else {
            return Cursor::IDLE;
        }
    case MOUSE_OVER:
        if (in_range) {
            if (clicked) {
                state = MOUSE_OVER;
                return Cursor::GRABBING;
            } else {
                state = MOUSE_OVER;
                return Cursor::CAN_GRAB;
            }
        } else {
            state = NORMAL;
            return Cursor::IDLE;
        }
    default:
        return Cursor::IDLE;
    }
}

std::int32_t SkillIcon::get_id() const noexcept
{
    return id;
}

UISkillbook::UISkillbook(const CharStats& in_stats,
                         const Skillbook& in_skillbook)
    : UIDragElement{{174, 20}},
      stats{in_stats},
      skillbook{in_skillbook},
      tab{0}
{
    nl::node main = nl::nx::ui["UIWindow2.img"]["Skill"]["main"];

    sprites.emplace_back(main["backgrnd"]);
    sprites.emplace_back(main["backgrnd2"]);
    sprites.emplace_back(main["backgrnd3"]);

    skill_d = main["skill0"];
    skill_e = main["skill1"];
    line = main["line"];

    auto tab_src = main["Tab"];
    nl::node tab_e = tab_src["enabled"];
    nl::node tab_d = tab_src["disabled"];

    for (std::uint16_t i = BT_TAB0; i <= BT_TAB4; ++i) {
        std::uint16_t tab_id = i - BT_TAB0;
        buttons[i]
            = std::make_unique<TwoSpriteButton>(tab_d[tab_id], tab_e[tab_id]);
    }
    for (std::uint16_t i = BT_SPUP0; i <= BT_SPUP3; ++i) {
        std::uint16_t spup_id = i - BT_SPUP0;
        Point<std::int16_t> spup_position
            = SKILL_OFFSET
              + Point<std::int16_t>{124, 20 + spup_id * ROW_HEIGHT};
        buttons[i]
            = std::make_unique<MapleButton>(main["BtSpUp"], spup_position);
    }

    book_text = {Text::A12M, Text::CENTER, Text::WHITE, "", 100};
    sp_label = {Text::A11M, Text::RIGHT, Text::LIGHTGREY};

    slider = {11, {92, 236}, 154, ROWS, 1, [this](bool upwards) {
                  std::int16_t shift = upwards ? -1 : 1;
                  bool above = offset + shift >= 0;
                  bool below = offset + 4 + shift <= skill_count;
                  if (above && below) {
                      change_offset(offset + shift);
                  }
              }};

    change_job(stats.get_stat(Maplestat::JOB));
    change_sp(stats.get_stat(Maplestat::SP));

    dimension = {174, 299};
}

void UISkillbook::draw(float alpha) const
{
    draw_sprites(alpha);

    book_icon.draw(position + Point<std::int16_t>{12, 85});
    book_text.draw(position + Point<std::int16_t>{100, 49});
    sp_label.draw(position + Point<std::int16_t>{162, 254});

    auto begin = icons.begin();
    if (icons.size() > offset) {
        begin = begin + offset;
    }

    auto end = icons.end();
    if (icons.size() > ROWS + offset) {
        end = begin + ROWS;
    }

    Point<std::int16_t> skill_position = position + SKILL_OFFSET;
    for (auto iter = begin; iter != end; ++iter) {
        skill_e.draw(skill_position);
        iter->first->draw(skill_position + ICON_OFFSET);
        if (iter != end - 1) {
            line.draw(skill_position + LINE_OFFSET);
        }

        skill_position.shift_y(ROW_HEIGHT);
    }

    draw_buttons(alpha);

    slider.draw(position);
}

Button::State UISkillbook::button_pressed(std::uint16_t id)
{
    switch (id) {
    case BT_TAB0:
    case BT_TAB1:
    case BT_TAB2:
    case BT_TAB3:
    case BT_TAB4:
        change_tab(id - BT_TAB0);
        return Button::PRESSED;
    case BT_SPUP0:
    case BT_SPUP1:
    case BT_SPUP2:
    case BT_SPUP3:
        send_spup(id - BT_SPUP0 + offset);
        return Button::PRESSED;
    default:
        return Button::PRESSED;
    }
}

void UISkillbook::double_click(Point<std::int16_t> cursor_pos)
{
    if (const SkillIcon* icon = icon_by_position(cursor_pos - position);
        icon) {
        std::int32_t skill_id = icon->get_id();
        std::int32_t skill_level = skillbook.get_level(skill_id);
        if (skill_level > 0) {
            Stage::get().get_combat().use_move(skill_id);
        }
    }
}

bool UISkillbook::remove_cursor(bool clicked, Point<std::int16_t> cursor_pos)
{
    if (UIDragElement::remove_cursor(clicked, cursor_pos)) {
        return true;
    }

    return slider.remove_cursor(clicked);
}

Cursor::State UISkillbook::send_cursor(bool clicked,
                                       Point<std::int16_t> cursor_pos)
{
    Cursor::State dstate = UIDragElement::send_cursor(clicked, cursor_pos);
    if (dragged) {
        return dstate;
    }

    Point<std::int16_t> cursor_relative = cursor_pos - position;
    if (slider.is_enabled()) {
        if (Cursor::State new_state
            = slider.send_cursor(cursor_relative, clicked);
            new_state) {
            clear_tooltip();
            return new_state;
        }
    }

    auto begin = icons.begin();
    if (icons.size() > offset) {
        begin = begin + offset;
    }

    auto end = icons.end();
    if (icons.size() > ROWS + offset) {
        end = begin + ROWS;
    }

    Point<std::int16_t> skill_position = position + SKILL_OFFSET;
    for (auto iter = begin; iter != end; ++iter) {
        auto normalized = cursor_pos - skill_position;
        if (Cursor::State state
            = iter->first->send_cursor(normalized, clicked);
            state) {
            switch (state) {
            case Cursor::GRABBING: {
                if (auto skill_id = iter->first->get_id();
                    skillbook.get_level(skill_id) > 0
                    && !SkillData::get(skill_id)
                            .is_passive()) { // Can't drag untrained skills or
                                             // passives
                    clear_tooltip();
                    iter->second->start_drag(normalized);
                    UI::get().drag_icon(iter->second.get());
                }
                break;
            }
            case Cursor::CAN_GRAB:
                show_skill(iter->first->get_id());
                break;
            default:
                break;
            }

            return state;
        }

        skill_position.shift_y(ROW_HEIGHT);
    }

    clear_tooltip();
    return Cursor::IDLE;
}

void UISkillbook::update_stat(Maplestat::Id stat, std::int16_t value)
{
    switch (stat) {
    case Maplestat::JOB:
        change_job(value);
        break;
    case Maplestat::SP:
        change_sp(value);
        break;
    default:
        return;
    }
}

void UISkillbook::update_skills(std::int32_t skill_id)
{
    if (skill_id / 10'000 == job.get_id()) {
        change_tab(tab);
    }
}

void UISkillbook::change_job(std::uint16_t id)
{
    job.change_job(id);

    Job::Level level = job.get_level();
    for (std::uint16_t i = 0; i <= Job::FOURTH; ++i) {
        buttons[BT_TAB0 + i]->set_active(i <= level);
    }

    change_tab(level - Job::BEGINNER);
}

void UISkillbook::change_sp(std::int16_t s)
{
    sp = s;
    sp_label.change_text(std::to_string(sp));

    change_offset(offset);
}

void UISkillbook::change_tab(std::uint16_t new_tab)
{
    buttons[BT_TAB0 + tab]->set_state(Button::NORMAL);
    buttons[BT_TAB0 + new_tab]->set_state(Button::PRESSED);
    tab = new_tab;

    icons.clear();
    skill_count = 0;

    Job::Level job_level = job_level_by_tab(tab);
    std::uint16_t sub_id = job.get_subjob(job_level);

    const JobData& data = JobData::get(sub_id);

    book_icon = data.get_icon();
    book_text.change_text(std::string{data.get_name()});

    for (std::int32_t skill_id : data.get_skills()) {
        std::int32_t level = skillbook.get_level(skill_id);
        std::int32_t master_level = skillbook.get_master_level(skill_id);

        bool invisible = SkillData::get(skill_id).is_invisible();
        if (invisible && master_level == 0) {
            continue;
        }

        auto skill_icon = std::make_unique<SkillIcon>(skill_id, level);
        icons.emplace_back(
            skill_icon.get(),
            std::make_unique<Icon>(
                std::move(skill_icon),
                SkillData::get(skill_id).get_icon(SkillData::NORMAL),
                -1));

        ++skill_count;
    }

    slider.set_rows(ROWS, skill_count);
    change_offset(0);
}

void UISkillbook::change_offset(std::uint16_t new_offset)
{
    offset = new_offset;

    for (std::int16_t i = 0; i < ROWS; ++i) {
        std::uint16_t index = BT_SPUP0 + i;
        std::uint16_t row = offset + i;
        buttons[index]->set_active(row < skill_count);
        if (row < icons.size()) {
            std::int32_t skill_id = icons[row].first->get_id();
            buttons[index]->set_state(can_raise(skill_id) ? Button::NORMAL
                                                          : Button::DISABLED);
        }
    }
}

void UISkillbook::show_skill(std::int32_t id)
{
    std::int32_t skill_id = id;
    std::int32_t level = skillbook.get_level(id);
    std::int32_t master_level = skillbook.get_master_level(id);
    std::int64_t expiration = skillbook.get_expiration(id);

    UI::get().show_skill(
        Tooltip::SKILL_BOOK, skill_id, level, master_level, expiration);
}

void UISkillbook::clear_tooltip()
{
    UI::get().clear_tooltip(Tooltip::SKILL_BOOK);
}

bool UISkillbook::can_raise(std::int32_t skill_id) const
{
    if (sp <= 0) {
        return false;
    }

    std::int32_t level = skillbook.get_level(skill_id);
    std::int32_t masterlevel = skillbook.get_master_level(skill_id);
    if (masterlevel == 0) {
        masterlevel = SkillData::get(skill_id).get_master_level();
    }

    if (level >= masterlevel) {
        return false;
    }

    switch (skill_id) {
    case SkillId::ANGEL_BLESSING:
        return false;
    default:
        return true;
    }
}

void UISkillbook::send_spup(std::uint16_t row)
{
    if (row >= icons.size()) {
        return;
    }

    std::int32_t skill_id = icons[row].first->get_id();
    SpendSpPacket{skill_id}.dispatch();

    /*
    if (skill_id < 10'000) { // Beginner skill
        auto& player = Stage::get().get_player();
        std::uint16_t new_sp = player.get_stats().get_stat(Maplestat::SP) - 1;

        player.get_stats().set_stat(Maplestat::SP, new_sp);
        update_stat(Maplestat::SP, new_sp);
    }
    */

    UI::get().disable();
}

Job::Level UISkillbook::job_level_by_tab(std::uint16_t t) const noexcept
{
    switch (t) {
    case 1:
        return Job::FIRST;
    case 2:
        return Job::SECOND;
    case 3:
        return Job::THIRD;
    case 4:
        return Job::FOURTH;
    default:
        return Job::BEGINNER;
    }
}

SkillIcon* UISkillbook::icon_by_position(Point<std::int16_t> cursor_pos)
{
    std::int16_t x = cursor_pos.x();
    if (x < SKILL_OFFSET.x() || x > 148) {
        return nullptr;
    }

    std::int16_t y = cursor_pos.y();
    if (y < SKILL_OFFSET.y()) {
        return nullptr;
    }

    std::uint16_t row = (y - SKILL_OFFSET.y()) / ROW_HEIGHT;
    if (row < 0 || row >= ROWS) {
        return nullptr;
    }

    std::uint16_t abs_row = offset + row;
    if (icons.size() <= abs_row) {
        return nullptr;
    }

    // auto iter = icons.begin() + abs_row;
    // return icons.data() + (iter - icons.begin());
    return icons[abs_row].first;
}
} // namespace jrc
