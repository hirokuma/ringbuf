#include "gtest/gtest.h"
//#include <string.h>
//#include "fff.h"
//DEFINE_FFF_GLOBALS;


extern "C" {
//評価対象本体
#include "ringbuf.c"
}

////////////////////////////////////////////////////////////////////////

class RingBuf: public testing::Test {
};

class RingBuf2: public testing::Test {
    void SetUp() {
        RingBufAlloc(&mBuf, 3UL);
        memset(mBuf.buf, 0xcc, mBuf.size);
    }

    void TearDown() {
        RingBufFree(&mBuf);
    }

public:
    static RingBuf_t mBuf;
};
RingBuf_t RingBuf2::mBuf;


////////////////////////////////////////////////////////////////////////

TEST_F(RingBuf, RingBufAlloc_Free)
{
    RingBuf_t buf;
    memset(&buf, 0, sizeof(buf));

    RingBufAlloc(&buf, 5UL);

    ASSERT_EQ(0, buf.rpos);
    ASSERT_EQ(0, buf.wpos);
    ASSERT_EQ(5UL, buf.size);
    ASSERT_NE((uint8_t *)0, buf.buf);
    ASSERT_TRUE(buf.empty);


    RingBufFree(&buf);
    
    ASSERT_EQ((uint8_t *)0, buf.buf);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite0)
{
    //最小-1
    //      +---+
    //     2|   |
    //     1|   |
    //   RW0|   |
    //      +---+
    const uint8_t DATA[] = { 1, };
    size_t sz = 0;

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
    ASSERT_EQ(0UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite1)
{
    //最小
    //      +---+
    //     2|   |
    //     1|   |
    //   RW0|   |
    //      +---+
    const uint8_t DATA[] = { 1, };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(1, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(1UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite2)
{
    //最大
    //      +---+
    //     2|   |
    //     1|   |
    //   RW0|   |
    //      +---+
    const uint8_t DATA[] = { 1, 2, 3 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(2, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(3, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(3UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite3)
{
    //最大+1
    //      +---+
    //     2|   |
    //     1|   |
    //   RW0|   |
    //      +---+
    const uint8_t DATA[] = { 1, 2, 3, 4 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(2, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(3, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(3UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite4)
{
    //空き上がは2つで、下に空きがない
    //      +---+
    //     2|   |
    //    W1|   |
    //    R0|   |
    //      +---+
    RingBuf2::mBuf.rpos = 0;
    RingBuf2::mBuf.wpos = 1;
    RingBuf2::mBuf.empty = false;

    const uint8_t DATA[] = { 4, 5 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(4, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(5, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(2UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite5)
{
    //空きがない
    //      +---+
    //     2|   |
    //   RW1|   |
    //     0|   |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 1;
    RingBuf2::mBuf.empty = false;

    const uint8_t DATA[] = { 6, 7 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(1, RingBuf2::mBuf.rpos);
    ASSERT_EQ(1, RingBuf2::mBuf.wpos);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(0UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite6)
{
    //空き上がは1つで、下に1つ
    //      +---+
    //    W2|   |
    //    R1|   |
    //     0|   |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 2;
    RingBuf2::mBuf.empty = false;

    const uint8_t DATA[] = { 5, 6 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(1, RingBuf2::mBuf.rpos);
    ASSERT_EQ(1, RingBuf2::mBuf.wpos);
    ASSERT_EQ(6, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(5, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(2UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufWrite7)
{
    //空き上がは1つで、下に1つ
    //      +---+
    //    W2|   |
    //    R1|   |
    //     0|   |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 2;
    RingBuf2::mBuf.empty = false;

    //下エリアの空きが足りない
    const uint8_t DATA[] = { 5, 6, 7 };
    size_t sz = sizeof(DATA);

    RingBufWrite(&RingBuf2::mBuf, DATA, &sz);

    ASSERT_EQ(1, RingBuf2::mBuf.rpos);
    ASSERT_EQ(1, RingBuf2::mBuf.wpos);
    ASSERT_EQ(6, RingBuf2::mBuf.buf[0]);
    ASSERT_EQ(0xcc, RingBuf2::mBuf.buf[1]);
    ASSERT_EQ(5, RingBuf2::mBuf.buf[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
    ASSERT_EQ(2UL, sz);
}


/////////////////////////////////////////////////

TEST_F(RingBuf2, RingBufRead_empty)
{
    //書込みデータ無し
    //      empty
    //      +---+
    //     2|   |
    //     1|   |
    //   RW0|   |
    //      +---+


    uint8_t data[5];
    memset(data, 0xdd, sizeof(data));
    size_t sz = sizeof(data);

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
    ASSERT_EQ(0UL, sz);
    for (int i = 0; i < (int)sizeof(data); i++) {
        ASSERT_EQ(0xdd, data[i]);
    }
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead0_0_1)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //     1| 2 |
    //   RW0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 0;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 1;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(1, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1UL, sz);
    ASSERT_EQ(1, data[0]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead0_0_2)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //     1| 2 |
    //   RW0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 0;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 2;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(2, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(2UL, sz);
    ASSERT_EQ(1, data[0]);
    ASSERT_EQ(2, data[1]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead0_0_3)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //     1| 2 |
    //   RW0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 0;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 3;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(3UL, sz);
    ASSERT_EQ(1, data[0]);
    ASSERT_EQ(2, data[1]);
    ASSERT_EQ(3, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead1_0_1)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //    R1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 1;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(2, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1UL, sz);
    ASSERT_EQ(2, data[0]);
    ASSERT_EQ(0xdd, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead1_0_2)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //    R1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 2;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(2UL, sz);
    ASSERT_EQ(2, data[0]);
    ASSERT_EQ(3, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead1_0_3)
{
    //書込みデータ
    //     !empty
    //      +---+
    //     2| 3 |
    //    R1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 1;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 3;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(2UL, sz);
    ASSERT_EQ(2, data[0]);
    ASSERT_EQ(3, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead2_0_1)
{
    //書込みデータ
    //     !empty
    //      +---+
    //    R2| 3 |
    //     1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 2;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 1;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1UL, sz);
    ASSERT_EQ(3, data[0]);
    ASSERT_EQ(0xdd, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead2_0_2)
{
    //書込みデータ
    //     !empty
    //      +---+
    //    R2| 3 |
    //     1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 2;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 2;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1UL, sz);
    ASSERT_EQ(3, data[0]);
    ASSERT_EQ(0xdd, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead2_0_3)
{
    //書込みデータ
    //     !empty
    //      +---+
    //    R2| 3 |
    //     1| 2 |
    //    W0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 2;
    RingBuf2::mBuf.wpos = 0;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 3;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    ASSERT_EQ(0, RingBuf2::mBuf.rpos);
    ASSERT_EQ(0, RingBuf2::mBuf.wpos);
    ASSERT_EQ(1UL, sz);
    ASSERT_EQ(3, data[0]);
    ASSERT_EQ(0xdd, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_TRUE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//RingBufRead_{rpos}_{wpos}_{sz}
TEST_F(RingBuf2, RingBufRead2_2_2)
{
    //書込みデータ
    //     !empty
    //      +---+
    //   RW2| 3 |
    //     1| 2 |
    //     0| 1 |
    //      +---+
    RingBuf2::mBuf.rpos = 2;
    RingBuf2::mBuf.wpos = 2;
    RingBuf2::mBuf.empty = false;
    RingBuf2::mBuf.buf[0] = 1;
    RingBuf2::mBuf.buf[1] = 2;
    RingBuf2::mBuf.buf[2] = 3;


    uint8_t data[3];
    memset(data, 0xdd, sizeof(data));
    size_t sz = 2;

    RingBufRead(&RingBuf2::mBuf, data, &sz);

    //下エリア用のバッファが足りないパターン
    ASSERT_EQ(1, RingBuf2::mBuf.rpos);
    ASSERT_EQ(2, RingBuf2::mBuf.wpos);
    ASSERT_EQ(2UL, sz);
    ASSERT_EQ(3, data[0]);
    ASSERT_EQ(1, data[1]);
    ASSERT_EQ(0xdd, data[2]);
    ASSERT_FALSE(RingBuf2::mBuf.empty);
}


/////////////////////////////////////////////////

//rpos/wposを同時にずらす
//      +---+
//     2|   |
//   RW1|   |
//     0|   |
//      +---+
//          writable : 3
//          readable : 0

TEST_F(RingBuf, RingBufRW1_1)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[3];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.rpos = ringbuf.wpos = i;        //ずらす

        //1回ずつ書いて、1回ずつ読む

        //書ける
        wbuf[0] = 0x12;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書ける
        wbuf[0] = 0x34;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書ける
        wbuf[0] = 0x56;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0x12, rbuf[0]);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0x34, rbuf[0]);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0x56, rbuf[0]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


/////////////////////////////////////////////////

TEST_F(RingBuf, RingBufRW1_2)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[5];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.rpos = ringbuf.wpos = i;        //ずらす

        //2回で書いて、1回で読む

        //書ける
        wbuf[0] = 0x12;
        wbuf[1] = 0x34;
        sz = 2;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(2UL, sz);

        //書ける
        wbuf[0] = 0x56;
        wbuf[1] = 0x78;
        sz = 2;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 5;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(3UL, sz);
        ASSERT_EQ(0x12, rbuf[0]);
        ASSERT_EQ(0x34, rbuf[1]);
        ASSERT_EQ(0x56, rbuf[2]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


/////////////////////////////////////////////////

TEST_F(RingBuf, RingBufRW1_3)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[5];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.rpos = ringbuf.wpos = i;        //ずらす

        //1回で書いて、2回で読む

        //書ける
        wbuf[0] = 0x12;
        wbuf[1] = 0x34;
        wbuf[2] = 0x56;
        sz = 3;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(3UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0x12, rbuf[0]);

        //読める
        sz = 5;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(2UL, sz);
        ASSERT_EQ(0x34, rbuf[0]);
        ASSERT_EQ(0x56, rbuf[1]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


/////////////////////////////////////////////////

TEST_F(RingBuf, RingBufRW1_4)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[5];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.rpos = ringbuf.wpos = i;        //ずらす

        //1回で書いて、1回で読む

        //書ける
        wbuf[0] = 0x12;
        wbuf[1] = 0x34;
        wbuf[2] = 0x56;
        sz = 3;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(3UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 5;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(3UL, sz);
        ASSERT_EQ(0x12, rbuf[0]);
        ASSERT_EQ(0x34, rbuf[1]);
        ASSERT_EQ(0x56, rbuf[2]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


/////////////////////////////////////////////////

//rpos = wpos + 1
//      +---+
//     2|   |
//    R1|   |
//    W0|   |
//      +---+
//          writable : 1
//          readable : 2

TEST_F(RingBuf, RingBufRW2_1)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[3];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.wpos = i;        //ずらす
        ringbuf.rpos = (ringbuf.wpos + 1) % 3;
        memset(ringbuf.buf, 0xdd, 3);
        ringbuf.empty = false;

        //1回ずつ書いて、1回ずつ読む

        //書ける
        wbuf[0] = 0x12;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0xdd, rbuf[0]);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0xdd, rbuf[0]);

        //読める
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(1UL, sz);
        ASSERT_EQ(0x12, rbuf[0]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


/////////////////////////////////////////////////

//          writable : 1
//          readable : 2
TEST_F(RingBuf, RingBufRW2_2)
{
    RingBuf_t   ringbuf;
    uint8_t     wbuf[3];
    uint8_t     rbuf[3];
    size_t      sz;

    for (int i = 0; i < 3; i++) {
        RingBufAlloc(&ringbuf, 3);
        ringbuf.wpos = i;        //ずらす
        ringbuf.rpos = (ringbuf.wpos + 1) % 3;
        memset(ringbuf.buf, 0xdd, 3);
        ringbuf.empty = false;

        //2回で書いて、1回で読む

        //少し書ける
        wbuf[0] = 0x12;
        wbuf[1] = 0x34;
        sz = 2;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(1UL, sz);

        //書けない
        wbuf[0] = 0x78;
        sz = 1;
        RingBufWrite(&ringbuf, wbuf, &sz);
        ASSERT_EQ(0UL, sz);

        //読める
        sz = 5;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(3UL, sz);
        ASSERT_EQ(0xdd, rbuf[0]);
        ASSERT_EQ(0xdd, rbuf[1]);
        ASSERT_EQ(0x12, rbuf[2]);

        //読めない
        sz = 1;
        RingBufRead(&ringbuf, rbuf, &sz);
        ASSERT_EQ(0UL, sz);

        RingBufFree(&ringbuf);
    }
}


