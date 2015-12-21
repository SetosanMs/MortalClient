/////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright � 2015 SYJourney                                               //
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Drop.h"
#include "Graphics\Animation.h"

namespace Gameplay
{
	using Graphics::Animation;

	class MesoDrop : public Drop
	{
	public:
		MesoDrop(int32_t oid, int32_t owner, vector2d<int16_t> start,
			vector2d<int16_t> dest, int8_t type, int8_t mode, const Animation* icon);

		void draw(const Camera& camera, float inter) const override;

	private:
		const Animation* icon;
	};
}