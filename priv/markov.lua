function markov_feed(filename)
    local markov = { starters = {}, mids = {}, ends = {} }
    for line in io.lines(filename) do
        local previous_word = nil
        for word in string.gmatch(line, "[%a!.?]+") do
            local sanitized = string.match(string.lower(word), "%a+")
            if sanitized ~= nil then
                local last = string.sub(word, -1)
                local ending = nil
                for _, p in pairs({ "!", ".", "?" }) do
                    if last == p then
                        ending = p
                    end
                end

                if previous_word == nil then
                    if markov.starters == nil then
                        markov.starters = { sanitized }
                    else
                        table.insert(markov.starters, sanitized)
                    end
                end

                if ending ~= nil then
                    -- List of punctuation
                    if markov.ends == nil then
                        markov.ends = { previous_word = { ending } }
                    elseif markov.ends[sanitized] == nil then
                        markov.ends[sanitized] = { ending }
                    else
                        table.insert(markov.ends[sanitized], ending)
                    end
                else
                    if markov.mids == nil then
                        markov.mids = { previous_word = { sanitized } }
                    elseif previous_word ~= nil then
                        -- We're in the middle of a sentence
                        local mid_list = markov.mids[previous_word]

                        if mid_list == nil then
                            -- We don't have the previous word
                            markov.mids[previous_word] = { sanitized }
                        else
                            -- We have the previous word
                            table.insert(markov.mids[previous_word], sanitized)
                        end
                    end
                end
                previous_word = sanitized
            end
        end
    end
    return markov
end

function markov_run(markov, min_words, max_words)
    local first = markov.starters[math.random(1, #markov.starters)]
    local string = string.upper(string.sub(first, 1, 1)) .. string.sub(first, 2)
    local current_word = first

    for i = 1, max_words do
        if i > min_words and markov.ends[current_word] ~= nil then
            local possible_ending = markov.ends[current_word]
            string = string .. possible_ending[math.random(#possible_ending)]
            return string
        else
            local word_table = markov.mids[current_word]
            current_word = word_table[math.random(#word_table)]
            string = string .. " " .. current_word
        end
    end

    return string
end
