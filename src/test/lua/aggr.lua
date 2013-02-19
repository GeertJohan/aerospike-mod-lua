
function even(s)
    
    local function _even(a)
        return a % 2 == 0
    end

    return s : filter(_even)
end

function increment(s)
    
    local function _map(a)
        return a + 1
    end

    return s : map(_map)
end

function sum(s)
    return s : reduce(math.sum)
end

function product(s)
    return s : reduce(math.product)
end

function rollup(s)

    local function _map(a)
        return map{ [a.campaign] = a.views }
    end

    local function _reduce(a, b)
        return map.merge(a, b, math.sum)
    end


    return s : map(_map) : reduce(_reduce)
end

function rollup2(s)

    local function _aggregate(a,b)
        a[b.campaign] = (a[b.campaign] or 0) + b.views
        return a
    end

    return s : aggregate(map(), _aggregate)
end