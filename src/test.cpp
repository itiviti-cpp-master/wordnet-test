#include "test_iterator.h"
#include "wordnet.h"

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <vector>

namespace {

class WordNetTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::ifstream synsets{"tests/etc/synsets.txt"}, hypernyms{"tests/etc/hypernyms.txt"};
        ASSERT_TRUE(synsets);
        ASSERT_TRUE(hypernyms);

        m_wordnet = std::make_unique<WordNet>(synsets, hypernyms);
        m_outcast = std::make_unique<Outcast>(*m_wordnet);
    }

    static void TearDownTestSuite()
    {
        m_wordnet.reset();
        m_outcast.reset();
    }

    WordNet::Nouns not_empty_container()
    {
        return m_wordnet->nouns();
    }

    using iterator_t = WordNet::Nouns::iterator;

    static std::unique_ptr<WordNet> m_wordnet;
    static std::unique_ptr<Outcast> m_outcast;
};

std::unique_ptr<WordNet> WordNetTest::m_wordnet;
std::unique_ptr<Outcast> WordNetTest::m_outcast;

//------------------------------------------------------------------------------

TEST_F(WordNetTest, nouns)
{
    std::istringstream synsets{R"(
82145,zombi zombie living_dead,a dead body that has been brought back to life by a supernatural force
82146,zombi zombie snake_god,a god of voodoo cults of African origin worshipped especially in West Indies  
)"};

    std::istringstream hypernyms;

    WordNet wordnet{synsets, hypernyms};
    EXPECT_TRUE(wordnet.is_noun("zombi"));
    EXPECT_TRUE(wordnet.is_noun("zombie"));
    EXPECT_TRUE(wordnet.is_noun("living_dead"));
    EXPECT_FALSE(wordnet.is_noun("zomb"));

    std::vector<std::string> nouns{wordnet.nouns().begin(), wordnet.nouns().end()};
    std::sort(nouns.begin(), nouns.end());

    EXPECT_EQ(nouns, (std::vector<std::string>{"living_dead", "snake_god", "zombi", "zombie"}));
}

//------------------------------------------------------------------------------

TEST_F(WordNetTest, distance)
{
    std::istringstream synsets{R"(
1,a a1,a gloss
2,b bd,b gloss
3,c,c gloss
4,d bd,d gloss
)"};

    std::istringstream hypernyms{R"(
2,1
3,1
4,2,3
)"};

    WordNet wordnet{synsets, hypernyms};

    EXPECT_EQ(wordnet.distance("a", "a"), 0);
    EXPECT_EQ(wordnet.distance("a", "a1"), 0);
    EXPECT_EQ(wordnet.distance("a", "b"), 1);
    EXPECT_EQ(wordnet.distance("b", "a"), 1);
    EXPECT_EQ(wordnet.distance("b", "c"), 2);

    EXPECT_EQ(wordnet.distance("d", "a"), 2);
    EXPECT_EQ(wordnet.distance("d", "c"), 1);

    EXPECT_EQ(wordnet.distance("bd", "c"), 1);
}

//---------------------------------------------------------------------------

TEST_F(WordNetTest, direction)
{
    std::istringstream synsets{R"(
1,a a1,a gloss
2,b bd,b gloss
3,c,c gloss
4,d bd,d gloss
5,e,e gloss
6,f,f gloss
)"};

    std::istringstream hypernyms{R"(
1,2,3
2,5
3,4
4,6
5,6
)"};

    WordNet wordnet{synsets, hypernyms};

    EXPECT_EQ(wordnet.distance("a", "a"), 0);
    EXPECT_EQ(wordnet.distance("a", "a1"), 0);
    EXPECT_EQ(wordnet.distance("a", "b"), 1);
    EXPECT_EQ(wordnet.distance("b", "a"), 1);
    EXPECT_EQ(wordnet.distance("b", "c"), 4);

    EXPECT_EQ(wordnet.distance("d", "a"), 2);
    EXPECT_EQ(wordnet.distance("d", "c"), 1);

    EXPECT_EQ(wordnet.distance("bd", "c"), 1);
}

//------------------------------------------------------------------------------

TEST_F(WordNetTest, direction1)
{
    std::istringstream synsets{R"(
1,a,a gloss
2,b,b gloss
3,c,c gloss
4,d,d gloss
5,f,d gloss
)"};

    std::istringstream hypernyms{R"(
3,1,2
2,1
5,2
4,3
)"};

    WordNet wordnet{synsets, hypernyms};

    EXPECT_EQ(wordnet.distance("d", "f"), 3);
}

//------------------------------------------------------------------------------

TEST_F(WordNetTest, unordered)
{
    std::istringstream synsets{R"(
123456,a a1,a gloss
25,b bd,b gloss
179,c,c gloss
2000000,d bd,d gloss
)"};

    std::istringstream hypernyms{R"(
25,123456
179,123456
2000000,25,179
)"};

    WordNet wordnet{synsets, hypernyms};

    EXPECT_EQ(wordnet.distance("a", "a"), 0);
    EXPECT_EQ(wordnet.distance("a", "a1"), 0);
    EXPECT_EQ(wordnet.distance("a", "b"), 1);
    EXPECT_EQ(wordnet.distance("b", "a"), 1);
    EXPECT_EQ(wordnet.distance("b", "c"), 2);

    EXPECT_EQ(wordnet.distance("d", "a"), 2);
    EXPECT_EQ(wordnet.distance("d", "c"), 1);

    EXPECT_EQ(wordnet.distance("bd", "c"), 1);
}

//------------------------------------------------------------------------------


TEST_F(WordNetTest, Basic)
{
    EXPECT_TRUE(m_wordnet->is_noun("whole-word_method"));
    EXPECT_TRUE(m_wordnet->is_noun("genus_Commiphora"));
    EXPECT_TRUE(m_wordnet->is_noun("barbershop_quartet"));

    EXPECT_EQ(std::distance(m_wordnet->nouns().begin(), m_wordnet->nouns().end()), 119188);
}

TEST_F(WordNetTest, Search)
{
    EXPECT_TRUE(m_wordnet->is_noun("Amazon"));
    EXPECT_EQ(m_wordnet->distance("Amazon", "Amazon"), 0);
    EXPECT_TRUE(m_wordnet->is_noun("application-oriented_language"));
    EXPECT_TRUE(m_wordnet->is_noun("problem-oriented_language"));
    EXPECT_EQ(m_wordnet->distance("application-oriented_language", "problem-oriented_language"), 0);
    EXPECT_TRUE(m_wordnet->is_noun("Alopius"));
    EXPECT_TRUE(m_wordnet->is_noun("Alosa"));
    EXPECT_EQ(m_wordnet->distance("Alopius", "Alosa"), 2);
    EXPECT_TRUE(m_wordnet->is_noun("Alpena"));
    EXPECT_TRUE(m_wordnet->is_noun("Appleton"));
    EXPECT_EQ(m_wordnet->distance("Alpena", "Appleton"), 2);
    EXPECT_TRUE(m_wordnet->is_noun("Aarhus"));
    EXPECT_EQ(m_wordnet->distance("Alpena", "Aarhus"), 2);
    EXPECT_TRUE(m_wordnet->is_noun("position"));
    EXPECT_EQ(m_wordnet->distance("Aarhus", "position"), 4);
    EXPECT_EQ(m_wordnet->distance("nose", "ear"), 3);
    EXPECT_EQ(m_wordnet->distance("ear", "nose"), 3);
    EXPECT_EQ(m_wordnet->distance("ware", "tinware"), 2);
    EXPECT_EQ(m_wordnet->distance("gesso", "defoliant"), 5);
}

TEST_F(WordNetTest, Outcast)
{
    EXPECT_EQ(m_outcast->outcast({"Turing"}), "");
    EXPECT_EQ(m_outcast->outcast({"Turing", "von_Neumann"}), "");
    EXPECT_EQ(m_outcast->outcast({"Turing", "von_Neumann", "Mickey_Mouse"}), "Mickey_Mouse");
    EXPECT_EQ(m_outcast->outcast({"horse", "zebra", "cat", "bear", "table"}), "table");
    EXPECT_EQ(m_outcast->outcast({"probability", "statistics", "mathematics", "physics"}), "probability");
    EXPECT_EQ(m_outcast->outcast({"earth", "fire", "air", "water", "heart"}), "heart");
    EXPECT_EQ(m_outcast->outcast({"Asia", "Australia", "North_America", "India", "Europe", "Antarctica", "South_America"}), "India");
    EXPECT_EQ(m_outcast->outcast({"water", "soda", "bed", "orange_juice", "milk", "apple_juice", "tea", "coffee"}), "bed");
    EXPECT_EQ(m_outcast->outcast({"Banti's_disease", "hyperadrenalism", "German_measles", "gargoylism", "Q_fever", "amebiosis", "anthrax", "playboy"}), "playboy");
    EXPECT_EQ(m_outcast->outcast({"apple", "orange", "banana", "grape", "strawberry", "cabbage", "mango", "watermelon"}), "cabbage");
    EXPECT_EQ(m_outcast->outcast({"car", "auto", "truck", "plane", "tree", "train", "vehicle", "van"}), "tree");
    EXPECT_EQ(m_outcast->outcast({"lumber", "wood", "tree", "leaf", "nail", "house", "building", "edifice", "structure"}), "tree");
    // EXPECT_EQ(m_outcast->outcast({"hair", "eyes", "arm", "mouth", "nose", "ear", "cheek", "brow", "chin"}), "arm");
    EXPECT_EQ(m_outcast->outcast({"cat", "cheetah", "dog", "wolf", "albatross", "horse", "zebra", "lemur", "orangutan", "chimpanzee"}), "albatross");
    EXPECT_EQ(m_outcast->outcast({"blue", "green", "yellow", "brown", "black", "white", "orange", "violet", "red", "serendipity"}), "serendipity");
    EXPECT_EQ(m_outcast->outcast({"apple", "pear", "peach", "banana", "lime", "lemon", "blueberry", "strawberry", "mango", "watermelon", "potato"}), "potato");
    EXPECT_EQ(m_outcast->outcast({"oak", "pine", "birch", "acacia"}), "");
}

TEST_F(WordNetTest, MultiThreadIteratorAccess)
{
    auto it = m_wordnet->nouns().begin();
    auto end = m_wordnet->nouns().end();

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

} // unnamed namespace
