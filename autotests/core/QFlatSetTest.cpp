#include <utils/qflatset.h>

#include <QTest>

class QFlatSetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test1();
};

void QFlatSetTest::test1()
{
    KActivities::QFlatSet<int, std::less<int>> set;
    set.insert(5);
    set.insert(4);
    set.insert(3);

    // Expect sorted
    auto expected = QList<int>{3, 4, 5};
    QCOMPARE(set, expected);

    {
        // try inserting a dupe, expect no change
        const auto [insertedIt, insertedIndex, didInsert] = set.insert(3);
        QCOMPARE(didInsert, false);
        QCOMPARE(insertedIndex, 0);
        QCOMPARE(insertedIt, set.begin());
        QCOMPARE(set, expected);
    }

    // try inserting something at the beginning, expect it to be inserted and have a valid iterator
    {
        const auto [insertedIt, insertedIndex, didInsert] = set.insert(0);
        QVERIFY(didInsert);
        QCOMPARE(insertedIt, set.begin());
        QCOMPARE(insertedIndex, (insertedIt - set.begin()));
    }

    set.clear();
    set.squeeze();
    QCOMPARE(set.capacity(), 0);

    for (int i = 0; i < 50; i++) {
        const auto [insertedIt, insertedIndex, didInsert] = set.insert(i);
        QVERIFY(didInsert);
        QCOMPARE(insertedIndex, (insertedIt - set.begin()));
        QCOMPARE(insertedIt, set.begin() + i);
    }
}

QTEST_MAIN(QFlatSetTest)

#include "QFlatSetTest.moc"
