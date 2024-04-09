local username = {}
local skin = {}
local time = {}
local full_rows = 0
local used_row = 0
local no_time = 1

--recieve data
addHook("PlayerMsg", function(s, type, t, m)
	if (m:sub(0, 5) == "SCTCL")
		local data_msg = m:sub(6)
		
		full_rows = full_rows + 1
		
		local first_comma = data_msg:find(",")
		local second_comma = data_msg:find(",", first_comma+1)
		
		username[full_rows] = data_msg:sub(0, first_comma-1)
		skin[full_rows] = data_msg:sub(first_comma+1, second_comma-1)
		time[full_rows] = data_msg:sub(second_comma+1)
		return true
	end
end)

--reset on map Load
addHook("MapLoad", function(mapnum)
	username = {}
	skin = {}
	time = {}
	full_rows = 0
	used_row = 0
end)


local function find_in_array(array, value)
	for k, v in ipairs(array) do
		if (v == value)
			return k
		end
	end
	return -1
end

--check player skin
addHook("PlayerThink", function(p)
	if (p == displayplayer)
		used_row = find_in_array(skin, p.mo.skin)
		
		if used_row == -1
			no_time = 1
		else
			no_time = 0
		end
	end
end)


local function show_score(v)
	if (no_time)
		v.drawString(4, 188, "\x87No best time found")
	else
		v.drawString(4, 188, "\x87BEST TIME FOR \x85"..skin[used_row]:upper().."\x87:")
		v.drawString(4, 192, "\x84"..time[used_row].."\x87 by \x82"..username[used_row])
	end
	
end
hud.add(show_score, "scores")
