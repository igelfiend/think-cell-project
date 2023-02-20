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
        The representation in the std::map must be canonical, that is, consecutive map entries must not contain the same value :
        ..., (3, 'A'), (5, 'A'), ... is not allowed. Likewise, the first entry in m_map must not contain the same value as m_valBegin.
        Initially, the whole range of K is associated with a given initial value, passed to the constructor of the interval_map<K, V> data structure.

        Key type K
            * besides being copyableand assignable, is less - than comparable via operator<, and
            * does not implement any other operations, in particular no equality comparison or arithmetic operators.
        Value type V
            * besides being copyable and assignable, is equality - comparable via operator==, and
            * does not implement any other operations.
        Many solutions we receive are incorrect. Consider using a randomized test to discover the cases that your implementation does not handle correctly.
        We recommend to implement a test function that tests the functionality of the interval_map, for example using a map of int intervals to char.

        Your task is to implement the function assign. Your implementation is graded by the following criteria in this order:

        Type requirements are met:
            You must adhere to the specification of the key and value type given above.
        Correctness: Your program should produce a working interval_map with the behavior described above.
            In particular, pay attention to the validity of iterators. It is illegal to dereference end iterators.
            Consider using a checking STL implementation such as the one shipped with Visual C++ or GCC.
        Canonicity:
            The representation in m_map must be canonical.
        Running time:
            Imagine your implementation is part of a library, so it should be big-O optimal.

        In addition:
            * Do not make big-O more operations on K and V than necessary because you do not know how fast operations on K/V are;
            remember that constructions, destructions and assignments are operations as well.
            * Do not make more than one operation of amortized O(log N), in contrast to O(1), running time, where N is the number of elements in m_map.
            Otherwise favor simplicity over minor speed improvements.
            * You should not take longer than 9 hours, but you may of course be faster. Do not rush, we would not give you this assignment if it were trivial.
    */

    // Assign value val to interval [keyBegin, keyEnd).
    // Overwrite previous values in this interval.
    // Conforming to the C++ Standard Library conventions, the interval
    // includes keyBegin, but excludes keyEnd.
    // If !( keyBegin < keyEnd ), this designates an empty interval,
    // and assign must do nothing.
    void assign(const K &keyBegin, const K &keyEnd, const V &val)
    {
        if (!(keyBegin < keyEnd))
        {
            return;
        }

        auto eq = [](auto v1, auto v2)
        {
            return (!(v1 < v2) && !(v2 < v1));
        };

        auto lastValue = m_valBegin;
        auto insertIterator = m_map.lower_bound(keyBegin);;
        bool isShouldFulfill = true;

        while (insertIterator != m_map.end())
        {
            if (insertIterator->first < keyEnd)
            {
                lastValue = insertIterator->second;
                insertIterator = m_map.erase(insertIterator);
            }
            // if we hit the end of some range - no fullfil required
            else
            {
                if (eq(insertIterator->first, keyEnd))
                {
                    isShouldFulfill = false;
                }

                break;
            }
        }

        m_map.insert(insertIterator, {keyBegin, val});
        if (isShouldFulfill)
        {
            m_map.insert(insertIterator, {keyEnd, lastValue});
        }

        // wipe out all duplicates
        auto prevValue = m_valBegin;
        for(auto it = m_map.begin(); it != m_map.end();)
        {
            if (it->second == prevValue)
            {
                it = m_map.erase(it);
            }
            else
            {
                prevValue = it->second;
                it++;
            }
        }
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

TEST(testIntervalMap, testInsertShouldMakeEmptyArray)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(4, 6, 'C');
    imap.assign(6, 8, 'D');

    imap.assign(2, 8, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "");
    EXPECT_EQ(imap.getValueSlice(0, 9), "AAAAAAAAA");
}

TEST(testIntervalMap, testInsertAbortedForIncorrectIndexesBeginIsBiggerThanEnd)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');

    imap.assign(8, 5, 'C');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][5, A]");
    EXPECT_EQ(imap.getValueSlice(0, 7), "AABBBAA");
}

TEST(testIntervalMap, testInsertAbortedForIncorrectIndexesBeginAndEndIsEqual)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 5, 'B');

    imap.assign(5, 5, 'C');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][5, A]");
    EXPECT_EQ(imap.getValueSlice(0, 7), "AABBBAA");
}

TEST(testIntervalMap, bunchOfSinglesRewriteAllWithDefault)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 3, 'B');
    imap.assign(3, 4, 'C');
    imap.assign(4, 5, 'D');
    imap.assign(5, 6, 'E');

    imap.assign(0, 10, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "");
    EXPECT_EQ(imap.getValueSlice(0, 12), "AAAAAAAAAAAA");
}

TEST(testIntervalMap, bunchOfSinglesRewriteAllWithDefaultMoreTight)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 3, 'B');
    imap.assign(3, 4, 'C');
    imap.assign(4, 5, 'D');
    imap.assign(5, 6, 'E');

    imap.assign(2, 6, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "");
    EXPECT_EQ(imap.getValueSlice(0, 12), "AAAAAAAAAAAA");
}

TEST(testIntervalMap, bunchOfSinglesRewriteAllWithNonDefault)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 3, 'B');
    imap.assign(3, 4, 'C');
    imap.assign(4, 5, 'D');
    imap.assign(5, 6, 'E');

    imap.assign(0, 10, 'F');
    EXPECT_EQ(imap.getMapSnippet(), "[0, F][10, A]");
    EXPECT_EQ(imap.getValueSlice(0, 11), "FFFFFFFFFFA");
}

TEST(testIntervalMap, InsertDefaultsInTheMiddleOfShortIntervalsWithExtraSpaceBetween)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(5, 7, 'C');
    imap.assign(8, 10, 'D');
    imap.assign(11, 13, 'E');
    // AABBACCADDAEEA

    imap.assign(2, 7, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "[8, D][10, A][11, E][13, A]");
    EXPECT_EQ(imap.getValueSlice(0, 14), "AAAAAAAADDAEEA");
}

TEST(testIntervalMap, InsertNonDefaultsInTheMiddleOfShortIntervalsWithExtraSpaceBetween)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(5, 7, 'C');
    imap.assign(8, 10, 'D');
    imap.assign(11, 13, 'E');
    // AABBACCADDAEEA

    imap.assign(2, 8, 'D');
    EXPECT_EQ(imap.getMapSnippet(), "[2, D][10, A][11, E][13, A]");
    EXPECT_EQ(imap.getValueSlice(0, 14), "AADDDDDDDDAEEA");
}

TEST(testIntervalMap, InsertDefaultRangeInTheMiddleOfEmptySpace)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(8, 10, 'D');

    imap.assign(5, 7, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, A][8, D][10, A]");
    EXPECT_EQ(imap.getValueSlice(0, 12), "AABBAAAADDAA");
}


TEST(testIntervalMap, InsertInDefaultRangeUpToEnd)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');
    imap.assign(5, 7, 'A');
    imap.assign(8, 10, 'D');
    imap.assign(11, 13, 'E');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, A][8, D][10, A][11, E][13, A]");
    EXPECT_EQ(imap.getValueSlice(0, 14), "AABBAAAADDAEEA");

    imap.assign(5, 13, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, A]");
    EXPECT_EQ(imap.getValueSlice(0, 14), "AABBAAAAAAAAAA");
}

TEST(testIntervalMap, InsertAtTheEndDefaultsWithSomeSpace)
{
    interval_map<int, char> imap{'A'};
    imap.assign(2, 4, 'B');

    imap.assign(5, 7, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "[2, B][4, A]");
    EXPECT_EQ(imap.getValueSlice(0, 8), "AABBAAAA");
}

TEST(testIntervalMap, InsertAtTheBeginDefaultsWithSomeSpace)
{
    interval_map<int, char> imap{'A'};
    imap.assign(5, 7, 'B');

    imap.assign(2, 4, 'A');
    EXPECT_EQ(imap.getMapSnippet(), "[5, B][7, A]");
    EXPECT_EQ(imap.getValueSlice(0, 8), "AAAAABBA");
}



