#include <iostream>
#include <gtest/gtest.h>
#include <map>
#include <sstream>


template<typename K, typename V>
class interval_map
{
private:
    V m_valBegin;
    std::map<K, V> m_map;

public:
    interval_map(const V &value)
        : m_valBegin(value)
    { }

    /*
        Each key-value-pair (k,v) in interval_map<K,V>::m_map means that the value v
        is associated with all keys from k (including) to the next key (excluding) in m_map.
        The member interval_map<K,V>::m_valBegin holds the value that is associated with all keys less than the first key in m_map.
        Example: Let M be an instance of interval_map<int,char> where

        M.m_valBegin=='A',
        M.m_map=={ (1,'B'), (3,'A') },
        Then M represents the mapping

        ...
        -2 -> 'A'
        -1 -> 'A'
        0 -> 'A'
        1 -> 'B'
        2 -> 'B'
        3 -> 'A'
        4 -> 'A'
        5 -> 'A'
        ...
    */
    // Assign value val to interval [keyBegin, keyEnd).
    // Overwrite previous values in this interval.
    // Conforming to the C++ Standard Library conventions, the interval
    // includes keyBegin, but excludes keyEnd.
    // If !( keyBegin < keyEnd ), this designates an empty interval,
    // and assign must do nothing.
    void assign(const K &keyBegin, const K &keyEnd, const V &value)
    {
//        std::cout << "from " << keyBegin << " to " << keyEnd << std::endl;

        if (!(keyBegin < keyEnd))
        {
//            std::cout << "incorrect ranges, return\n";
            return;
        }

        auto first_gte_range_end = m_map.lower_bound(keyEnd);
        if (first_gte_range_end == m_map.begin())
        {
            /*
                Handle cases:
                    * empty map
                    * new range goes at the beginning and not intercepts with existed one
            */

//            std::cout << "not found -> just insert default\n";
            m_map.insert(m_map.end(), {keyBegin, value});
            m_map.insert(m_map.end(), {keyEnd, m_valBegin});
            return;
        }

        auto first_gte_range_start = m_map.lower_bound(keyBegin);
        if (first_gte_range_start == m_map.end())
        {
            /*
              Handles cases when new range is at the end of ranges and didnt intercepts and has extra space
            */
            m_map.insert(m_map.end(), {keyBegin, value});
            m_map.insert(m_map.end(), {keyEnd, m_valBegin});
            return;
        }

        auto lastValue = m_valBegin;
        auto insertIterator = first_gte_range_start;
        bool shouldFulfill = true;

        while (insertIterator != m_map.end())
        {
            if (insertIterator->first < keyEnd)
            {
                lastValue = insertIterator->second;
                insertIterator = m_map.erase(insertIterator);
            }
            // if we hit the end of some range- no fullfil required
            else if (insertIterator->first == keyEnd)
            {
                shouldFulfill = false;
                break;
            }
            else
            {
                break;
            }
        }
        auto rangeBeginIterator = m_map.insert(insertIterator, {keyBegin, value});

        if (shouldFulfill)
        {
            m_map.insert(insertIterator, {keyEnd, lastValue});
        }

        auto leftElementIterator = rangeBeginIterator;
        leftElementIterator--;

        auto rightElementIterator = rangeBeginIterator;
        rightElementIterator++;

        // process left repeat elements
        if (rangeBeginIterator != m_map.begin())
        {
//            auto leftElementIterator = rangeBeginIterator;
//            leftElementIterator--;
            if (leftElementIterator->second == rangeBeginIterator->second)
            {
                std::cout << "left element: " << leftElementIterator->first << " -> " << leftElementIterator->second << std::endl;
                std::cout << "erase left duplicates\n";
                m_map.erase(rangeBeginIterator);
            }
        }

        // process right repeat elements
//        auto rightElementIterator = rangeBeginIterator;
//        rightElementIterator++;

        if (rightElementIterator->second == rangeBeginIterator->second)
        {
            std::cout << "right element: " << rightElementIterator->first << " -> " << rightElementIterator->second << std::endl;
            std::cout << "erase right duplicates\n";
            m_map.erase(rightElementIterator);
        }
    }

    bool eq(const K &v1, const K &v2) const
    {
        return (!(v1 < v2) && !(v2 < v1));
    }

    const V &operator[](const K &key) const
    {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin()) {
            return m_valBegin;
        }
        else {
            return (--it)->second;
        }
    }

    std::string getMapSnippet() const
    {
        std::stringstream stream;
        for(const auto &[key, value]: m_map)
        {
            stream << "[" << key << ", " << value << "]";
        }
        return stream.str();
    }

    std::string getDataSlice(const K &keyBegin, const K &keyEnd) const
    {
        std::stringstream stream;
        for(auto i = keyBegin; i < keyEnd; i++)
        {
            stream << i << " -> " << (*this)[i] << "\n";
        }

        return stream.str();

    }

    std::string getValueSlice(const K &keyBegin, const K &keyEnd) const
    {
        std::stringstream stream;
        for(auto i = keyBegin; i < keyEnd; i++)
        {
            stream << (*this)[i];
        }

        return stream.str();
    }
};


TEST(testIntervalMap, testItemGetFromEmptyMap)
{
    interval_map<int, char> imap{'A'};

    EXPECT_EQ(imap.getMapSnippet(), "");
    EXPECT_EQ(imap.getValueSlice(-4, 5), "AAAAAAAAA");
}

TEST(testIntervalMap, testItemFromSimpleMap)
{
    interval_map<int, char> imap{'A'};

    imap.assign(2, 5, 'B');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][5, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AABBBAAAA");
}

TEST(testIntervalMap, testInsertRangeAtTheBeginWithoutOverlap)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');

    imap.assign(-2, 0, 'C');
    EXPECT_EQ(imap.getMapSnippet(), "[-2, C][0, A][2, B][4, A]");
    EXPECT_EQ(imap.getValueSlice(-4, 5), "AACCAABBA");
}

TEST(testIntervalMap, testInsertRangeAtTheEndWithoutOverlapAndWithExtraSpace)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');

    imap.assign(5, 6, 'C');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, A][5, C][6, A]");
    EXPECT_EQ(imap.getValueSlice(0, 8), "AABBACAA");
}

TEST(testIntervalMap, testInsertRangeAtTheEndWithoutOverlap)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');

    imap.assign(5, 8, 'C');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][5, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AABBBCCCA");
}

TEST(testIntervalMap, testInsertRangeAtTheBeginWithSingleRangeIntersect)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(0, 3, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[0, D][3, B][5, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "DDDBBCCCA");
}

TEST(testIntervalMap, testInsertWithinRange)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(2, 3, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, D][3, B][5, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AADBBCCCA");
}

TEST(testIntervalMap, testInsertWithMultipleErase)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(0, 6, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[0, D][6, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "DDDDDDCCA");
}

TEST(testIntervalMap, testInsertWithIntersectionFor2Ranges)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(4, 6, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, D][6, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AABBDDCCA");
}

TEST(testIntervalMap, testInsertAtTheEndWithIntersect)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(6, 9, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][5, C][6, D][9, A]");
    EXPECT_EQ(imap.getValueSlice(0, 10), "AABBBCDDDA");
}

TEST(testIntervalMap, testInsertWithRangeReplace)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');
    imap.assign(5, 8, 'C');

    imap.assign(2, 5, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, D][5, C][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AADDDCCCA");
}

TEST(testIntervalMap, testInsertShouldEraseLeftDuplicates)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(4, 6, 'C');
    imap.assign(6, 8, 'D');

    imap.assign(4, 6, 'B');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][6, D][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 10), "AABBBBDDAA");
}

TEST(testIntervalMap, testInsertShouldEraseRightDuplicates)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(4, 6, 'C');
    imap.assign(6, 8, 'D');

    imap.assign(4, 6, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, D][8, A]");
    EXPECT_EQ(imap.getValueSlice(0, 10), "AABBDDDDAA");
}

TEST(testIntervalMap, testInsertShouldEraseBothSidesDuplicatesAndMapWontBeEmpty)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(4, 6, 'C');
    imap.assign(6, 8, 'B');
    imap.assign(8, 10, 'D');

    imap.assign(4, 6, 'B');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][8, D][10, A]");
    EXPECT_EQ(imap.getValueSlice(0, 12), "AABBBBBBDDAA");
}



