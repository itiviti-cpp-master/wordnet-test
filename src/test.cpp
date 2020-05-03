#include <gtest/gtest.h>
#include "wordnet.h"

class WordNetTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    { m_wordnet = new WordNet("etc/synsets.txt", "etc/hypernyms.txt"); }

    static void TearDownTestSuite()
    {
        delete m_wordnet;
        m_wordnet = nullptr;
    }

    static WordNet * m_wordnet;
};

WordNet * WordNetTest::m_wordnet = nullptr;

TEST_F(WordNetTest, Basic)
{
    ASSERT_TRUE(m_wordnet->is_noun("whole-word_method"));
    ASSERT_TRUE(m_wordnet->is_noun("genus_Commiphora"));
    ASSERT_TRUE(m_wordnet->is_noun("barbershop_quartet"));

    auto it = m_wordnet->nouns();
    auto end = m_wordnet->end();
    ASSERT_TRUE(it != end);
    size_t n = 0;
    while (it != end) {
        ++n;
        ++it;
    }

    ASSERT_TRUE(146547 == n);
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
