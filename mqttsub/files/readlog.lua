#!/usr/bin/lua

local argparse = require "argparse"
local sqlite3 = require("lsqlite3")
local db

function file_exists(name)
  local f = io.open(name, "r")
  if f ~= nil then 
    io.close(f) 
    return true 
  else 
    return false 
  end
end

local function opendb(name)
  if not file_exists(name) then
    print("Error: Cannot open database.")
    os.exit(1)
  end
  db = sqlite3.open(name)
end

local parser = argparse()
parser:option("--date_from", "Start date (ex. 2025-01-01 00:00:00 or 2025-01-01)")
parser:option("--date_to", "End date (ex. 2025-01-01 00:00:00 or 2025-01-01)")
parser:option("--topics", "Comma-separated list of topics to filter by")

local args = parser:parse()

local function is_valid_date(str)
  local y, m, d, h, min, sec = str:match("(%d+)-(%d+)-(%d+) (%d+):(%d+):(%d+)")
  if not h then
    y, m, d = str:match("(%d+)-(%d+)-(%d+)")
    h, min, sec = 0, 0, 0
  end
  if not y or not m or not d then return false end
  y, m, d, h, min, sec = tonumber(y), tonumber(m), tonumber(d), tonumber(h), tonumber(min), tonumber(sec)
  
  if d <= 0 or d > 31 or m <= 0 or m > 12 or y <= 0 then return false end
  if m == 4 or m == 6 or m == 9 or m == 11 then 
    if d > 30 then return false end
  elseif m == 2 then
    if y % 400 == 0 or (y % 100 ~= 0 and y % 4 == 0) then
      if d > 29 then return false end
    else
      if d > 28 then return false end
    end
  else
    if d > 31 then return false end
  end

  if h < 0 or h > 23 or min < 0 or min > 59 or sec < 0 or sec > 59 then return false end

  return true
end

local function createDateFilterQuery(date_from, date_to, sql_select)
  if date_from or date_to then sql_select = sql_select.." sent_at " end
  if date_from and date_to then
    sql_select = sql_select..'BETWEEN "' .. date_from .. '" AND "' .. date_to .. '"'
  elseif date_from then
    sql_select = sql_select..'>= "' .. date_from .. '"'
  elseif date_to then
    sql_select = sql_select..'<= "' .. date_to .. '"'
  end
  return sql_select
end

local function addTopicsToQuery(sql_select, topics)
  if #topics == 0 then
    return sql_select
  end
  
  for i = 1, #topics do
    sql_select = sql_select .. ' topic = "' .. topics[i] .. '"'
    if i < #topics then
      sql_select = sql_select .. ' OR '
    end
  end
  return sql_select
end

local function appendFiltersToQuery(sql_select, date_from, date_to, topics)
  if date_from or date_to then
    sql_select = createDateFilterQuery(date_from, date_to, sql_select)
    sql_select = sql_select .. ' AND'
  end
  sql_select = addTopicsToQuery(sql_select, topics)
  return sql_select
end

local function createSQLQuery(sql_select, date_from, date_to, topics)
  local sql_select = 'SELECT * FROM mqtt_messages'
  if date_from or date_to or #topics>0 then 
    sql_select = sql_select..' WHERE'
  end
  sql_select = appendFiltersToQuery(sql_select, date_from, date_to, topics)

  return sql_select
end

opendb('/log/topics.db')
local topics = {}
if args.topics then
  for topic in args.topics:gmatch("([^ ]+)") do
    table.insert(topics, topic)
  end
end

local date_from = args.date_from
local date_to = args.date_to
local sql_select = ""

sql_select = createSQLQuery(sql_select, date_from, date_to, topics)


local stmt = db:prepare(sql_select)

if stmt then
  local result = stmt:step()
  if result == sqlite3.ROW then
    repeat
      local id = stmt:get_value(0)
      local topic = stmt:get_value(1)
      local payload = stmt:get_value(2)
      local sent_at = stmt:get_value(3)

      print('ID = '..id)
      print('Topic = '..topic)
      print('Payload = '..payload)
      print('Sent at = '..sent_at)
      print('\n')

      result = stmt:step()
    until result ~= sqlite3.ROW
  else
    print("No results found.")
  end
  stmt:finalize()
else
  print("SQL Error: Could not prepare statement")
end

db:close()