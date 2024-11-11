--Script by Meziu & LeonardoTheMutant

local username, skin, time, time_str, recieved_row, used_row, got_time

local function SCTCL_ResetBuffer()
	username = {} --username (string)
	skin = {} --skin (string)
	time = {} --time (integer, tics value)
	time_str = {} --time (string, tics value converted to mm:ss.cc)
	recieved_row = 0 --the row number being currently procecced by the reciever
	used_row = -1 --row used by the current player
end

--
-- The "SCTCL" reciever
-- ("Server C To Client Lua")
--
addHook("PlayerMsg", function(s, t, r, m) --source, type, reciever (target), message
	if (m:sub(0, 6) == "#SCTCL") --'#' to make it more rare for players to accidentaly type in a conversation
		if (used_row != -1) then SCTCL_ResetBuffer() end --if the data is being sent again clear the buffer first

		local data_msg = m:sub(7)
		
		recieved_row = $ + 1
		
		local first_comma = data_msg:find(",")
		local second_comma = data_msg:find(",", (first_comma + 1))
		local third_comma = data_msg:find(",", (second_comma + 1))
		
		username[recieved_row] = data_msg:sub(0, (first_comma - 1))
		skin[recieved_row] = data_msg:sub((first_comma + 1), (second_comma-1))
		time[recieved_row] = data_msg:sub((second_comma + 1), (third_comma-1)) --make sure you always check this variable for NIL!
		time_str[recieved_row] = data_msg:sub(third_comma + 1)
		return true
	end
end)

addHook("MapLoad", SCTCL_ResetBuffer)
addHook("PlayerJoin", SCTCL_ResetBuffer)

local function find_in_array(array, value)
	for k, v in ipairs(array) do
		if v == value
			return k
		end
	end
	return -1
end

addHook("PlayerThink", function(p) --player
	if (p.mo) and (p.mo.valid) and (skin)
		used_row = find_in_array(skin, p.mo.skin)
	end
end)


local function show_score(v)
	if (used_row != -1) --there is data suitable for us (the player)
		v.drawString(4, 176, "BEST TIME FOR "..string.upper(skin[used_row])..":", 45056)
		v.drawString(4, 184, time_str[used_row].." by "+username[used_row])
		--server also sends the time value in tics in case you need
		v.drawString(160, 184, "("..time[used_row].." tics)")
	else --we got no data for us
		v.drawString(4, 173, "NO BEST TIME YET, BE FIRST TO FINISH!", 45056)
	end
end
hud.add(show_score, "scores")
