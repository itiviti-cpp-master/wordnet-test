#include <gtest/gtest.h>
#include "wordnet.h"
#include "test_iterator.h"

class WordNetTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_wordnet = new WordNet("tests/etc/synsets.txt", "tests/etc/hypernyms.txt"); 
        m_outcast = new Outcast(*m_wordnet);
    }

    static void TearDownTestSuite()
    {
        delete m_wordnet;
        m_wordnet = nullptr;
        delete m_outcast;
        m_outcast = nullptr;
    }

    WordNet & not_empty_container()
    {
        return *m_wordnet;
    }

    using iterator_t = WordNet::iterator;
    static WordNet * m_wordnet;
    static Outcast * m_outcast;
};

WordNet * WordNetTest::m_wordnet = nullptr;
Outcast * WordNetTest::m_outcast = nullptr;

TEST_F(WordNetTest, Basic)
{
    ASSERT_TRUE(m_wordnet->is_noun("whole-word_method"));
    ASSERT_TRUE(m_wordnet->is_noun("genus_Commiphora"));
    ASSERT_TRUE(m_wordnet->is_noun("barbershop_quartet"));

    auto it = m_wordnet->begin();
    auto end = m_wordnet->end();
    ASSERT_TRUE(it != end);
    size_t n = 0;
    while (it != end) {
        ++n;
        ++it;
    }

    ASSERT_TRUE(119188 == n);
}

TEST_F(WordNetTest, Search)
{
    ASSERT_TRUE(m_wordnet->is_noun("Amazon"));
    ASSERT_TRUE(m_wordnet->distance("Amazon", "Amazon") == 0);
    ASSERT_TRUE(m_wordnet->is_noun("application-oriented_language"));
    ASSERT_TRUE(m_wordnet->is_noun("problem-oriented_language"));
    ASSERT_TRUE(m_wordnet->distance("application-oriented_language", "problem-oriented_language") == 0);
    ASSERT_TRUE(m_wordnet->is_noun("Alopius"));
    ASSERT_TRUE(m_wordnet->is_noun("Alosa"));
    ASSERT_TRUE(m_wordnet->distance("Alopius", "Alosa") == 2);
    ASSERT_TRUE(m_wordnet->is_noun("Alpena"));
    ASSERT_TRUE(m_wordnet->is_noun("Appleton"));
    ASSERT_TRUE(m_wordnet->distance("Alpena", "Appleton") == 2);
    ASSERT_TRUE(m_wordnet->is_noun("Aarhus"));
    ASSERT_TRUE(m_wordnet->distance("Alpena", "Aarhus") == 2);
    ASSERT_TRUE(m_wordnet->is_noun("position"));
    ASSERT_TRUE(m_wordnet->distance("Aarhus", "position") == 4);
}

TEST_F(WordNetTest, Outcast)
{
    std::vector<std::string> d1 {"Turing"};
    ASSERT_TRUE(m_outcast->outcast(d1).empty());

    std::vector<std::string> d2 {"Turing", "von_Neumann"};
    ASSERT_TRUE(m_outcast->outcast(d2).empty());

    std::vector<std::string> d3 {"Turing", "von_Neumann", "Mickey_Mouse"};
    ASSERT_TRUE(m_outcast->outcast(d3) == "Mickey_Mouse");

    std::vector<std::string> d5 {"horse", "zebra", "cat", "bear", "table"};
    ASSERT_TRUE(m_outcast->outcast(d5) == "table");

    std::vector<std::string> d4 {"probability", "statistics", "mathematics", "physics"};
    ASSERT_TRUE(m_outcast->outcast(d4) == "probability");

    std::vector<std::string> d6 {"earth", "fire", "air", "water", "heart"};
    ASSERT_TRUE(m_outcast->outcast(d6) == "heart");

    std::vector<std::string> d7 {"Asia", "Australia", "North_America", "India", "Europe", "Antarctica", "South_America"};
    ASSERT_TRUE(m_outcast->outcast(d7) == "India");

    std::vector<std::string> d8 {"water", "soda", "bed", "orange_juice", "milk", "apple_juice", "tea", "coffee"};
    ASSERT_TRUE(m_outcast->outcast(d8) == "bed");

    std::vector<std::string> d9 {"Banti's_disease", "hyperadrenalism", "German_measles", "gargoylism", "Q_fever", "amebiosis", "anthrax", "playboy"};
    ASSERT_TRUE(m_outcast->outcast(d9) == "playboy");

    std::vector<std::string> d10 {"apple", "orange", "banana", "grape", "strawberry", "cabbage", "mango", "watermelon"};
    ASSERT_TRUE(m_outcast->outcast(d10) == "cabbage");

    std::vector<std::string> d11 {"car", "auto", "truck", "plane", "tree", "train", "vehicle", "van"};
    ASSERT_TRUE(m_outcast->outcast(d11) == "tree");

    std::vector<std::string> d12 {"lumber", "wood", "tree", "leaf", "nail", "house", "building", "edifice", "structure"};
    ASSERT_TRUE(m_outcast->outcast(d12) == "tree");

    /*
    std::vector<std::string> d13 {"hair", "eyes", "arm", "mouth", "nose", "ear", "cheek", "brow", "chin"};
    ASSERT_TRUE(m_outcast->outcast(d13) == "arm");
    */

    std::vector<std::string> d14 {"cat", "cheetah", "dog", "wolf", "albatross", "horse", "zebra", "lemur", "orangutan", "chimpanzee"};
    ASSERT_TRUE(m_outcast->outcast(d14) == "albatross");

    std::vector<std::string> d15 {"blue", "green", "yellow", "brown", "black", "white", "orange", "violet", "red", "serendipity"};
    ASSERT_TRUE(m_outcast->outcast(d15) == "serendipity");

    std::vector<std::string> d16 {"apple", "pear", "peach", "banana", "lime", "lemon", "blueberry", "strawberry", "mango", "watermelon", "potato"};
    ASSERT_TRUE(m_outcast->outcast(d16) == "potato");

    // Extra test coverage by @dedlocc
    ASSERT_TRUE(m_wordnet->distance("nose", "ear") == 3);
    ASSERT_TRUE(m_wordnet->distance("ear", "nose") == 3);
    ASSERT_TRUE(m_wordnet->distance("ware", "tinware") == 2);
    ASSERT_TRUE(m_wordnet->distance("gesso", "defoliant") == 5);

    std::vector<std::string> d17 {"oak", "pine", "birch", "acacia"};
    ASSERT_TRUE(m_outcast->outcast(d17).empty());
    //
}

TEST_F(WordNetTest, MultiThreadIteratorAccess)
{
    auto it = m_wordnet->begin();
    auto end = m_wordnet->end();

    std::vector<iterator_test::Job<iterator_t>> jobs;
    size_t count = 10;
    for (size_t i = 0; i < count; ++i) {
        jobs.emplace_back([it, end]() -> std::pair<iterator_t, iterator_t> { return {it, end}; }, 
                            iterator_test::test_multipass<iterator_t>);
    }
    iterator_test::run_multithread<iterator_t>(jobs, 10);
}

using TypesToTest = ::testing::Types<WordNetTest>;
INSTANTIATE_TYPED_TEST_SUITE_P(WN, IteratorTest, TypesToTest);
