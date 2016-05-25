/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 hirokuma
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "ringbuf.h"

#define M_ROUND(pos,size,n)     ((((pos)+(n)) >= (size)) ? (pos)+(n)-(size) : (pos)+(n))
#define M_WPOS_ROUND(pBuf,n)    M_ROUND((pBuf)->wpos, (pBuf)->size, (n))
#define M_RPOS_ROUND(pBuf,n)    M_ROUND((pBuf)->rpos, (pBuf)->size, (n))

//#define DBG_PRINTF          printf
#define DBG_PRINTF(...)     /*none*/
#define DBG_STATUS(pBuf)    {   \
        DBG_PRINTF("[%s %d]rpos=%d, wpos=%d, size=%d, EMPTY[%d]\n", __func__, __LINE__, pBuf->rpos, pBuf->wpos, pBuf->size, pBuf->empty);  \
    }


void RingBufAlloc(RingBuf_t *pBuf, size_t BufSz)
{
    pBuf->buf = (uint8_t *)malloc(BufSz);
    pBuf->size = BufSz;
    RingBufReset(pBuf);
}


void RingBufFree(RingBuf_t *pBuf)
{
    free(pBuf->buf);
    pBuf->buf = NULL;
}


void RingBufReset(RingBuf_t *pBuf)
{
    pBuf->rpos = 0;
    pBuf->wpos = 0;
    pBuf->empty = true;
    //memset(mReadBuf, 0xcc, pBuf->size);
}


void RingBufWrite(RingBuf_t *pBuf, const uint8_t *pData, size_t *pSize)
{
    DBG_STATUS(pBuf);

    if ((*pSize == 0) || ((pBuf->rpos == pBuf->wpos) && !pBuf->empty)) {
        //何もしない
        *pSize = 0;
    }
    else if (pBuf->rpos > pBuf->wpos) {
        //rposが同じか先
        ssize_t esize =  pBuf->rpos - pBuf->wpos;
        DBG_PRINTF("[%s %d]*pSize=%d, esize=%d\n", __func__, __LINE__, *pSize, esize);
        if (esize < (ssize_t)*pSize) {
            //空きが足りない
            *pSize = esize;
        }
        DBG_PRINTF("[%s %d]*pSize=%d, esize=%d\n", __func__, __LINE__, *pSize, esize);
        memcpy(pBuf->buf + pBuf->wpos, pData, *pSize);
        pBuf->wpos = M_WPOS_ROUND(pBuf, *pSize);
    }
    else {
        //wposが先

        //エリアの空き
        ssize_t esize1 = pBuf->size - pBuf->wpos;   //上エリア
        ssize_t esize2 = pBuf->rpos;                //下エリア
        DBG_PRINTF("[%s %d]*pSize=%d, esize1=%d, esize2=%d\n", __func__, __LINE__, *pSize, esize1, esize2);

        if ((ssize_t)*pSize <= esize1) {
            //上エリアだけで足りる
            memcpy(pBuf->buf + pBuf->wpos, pData, *pSize);
            pBuf->wpos = M_WPOS_ROUND(pBuf, *pSize);
            DBG_PRINTF("  write upper only: *pSize=%d\n", *pSize);
        }
        else {
            //上エリアはすべて使う
            memcpy(pBuf->buf + pBuf->wpos, pData, esize1);
            DBG_PRINTF("  write upper: esize1=%d\n", esize1);

            //残り書込みサイズ
            *pSize -= esize1;

            if (esize2 > 0) {
                //下エリアあり
                if (esize2 < (ssize_t)*pSize) {
                    //空きが足りない
                    *pSize = esize2;
                    DBG_PRINTF("[%s %d]*pSize=%d\n", __func__, __LINE__, *pSize);
                }
                memcpy(pBuf->buf, pData + esize1, *pSize);
                DBG_PRINTF("  write lower: *pSize=%d\n", *pSize);

                *pSize += esize1;
                pBuf->wpos = M_WPOS_ROUND(pBuf, *pSize);
            }
            else {
                //下エリアに空きがない
                pBuf->wpos = 0;
                *pSize = esize1;
                DBG_PRINTF("[%s %d]*pSize=%d\n", __func__, __LINE__, *pSize);
            }
        }
    }
    if (*pSize > 0) {
        pBuf->empty = false;
    }
    DBG_PRINTF("[%s %d]*pSize=%d\n", __func__, __LINE__, *pSize);
    DBG_STATUS(pBuf);
}


void RingBufRead(RingBuf_t *pBuf, uint8_t *pData, size_t *pSize)
{
    DBG_STATUS(pBuf);

    if (pBuf->empty) {
        //受信済みデータ無し
        *pSize = 0;
    }
    if (pBuf->rpos < pBuf->wpos) {
        //wposが先
        ssize_t rsize = pBuf->wpos - pBuf->rpos;
        DBG_PRINTF("[%s %d]*pSize=%d, rsize=%d\n", __func__, __LINE__, *pSize, rsize);
        if ((ssize_t)*pSize < rsize) {
            rsize = *pSize;
        }
        DBG_PRINTF("[%s %d]*pSize=%d, rsize=%d\n", __func__, __LINE__, *pSize, rsize);
        memcpy(pData, pBuf->buf + pBuf->rpos, rsize);
        pBuf->rpos += rsize;
        *pSize = rsize;
    }
    else {
        //rposが先
        
        //上エリア
        ssize_t rsize1 = pBuf->size - pBuf->rpos;   //上エリア
        ssize_t rsize2 = pBuf->wpos;                //下エリア
        DBG_PRINTF("[%s %d]*pSize=%d, rsize1=%d, rsize2=%d\n", __func__, __LINE__, *pSize, rsize1, rsize2);

        if ((ssize_t)*pSize <= rsize1) {
            //上エリアだけで足りる
            memcpy(pData, pBuf->buf + pBuf->rpos, *pSize);
            pBuf->rpos = M_RPOS_ROUND(pBuf, *pSize);
            DBG_PRINTF("  read upper only: *pSize=%d\n", *pSize);
        }
        else {
            //上エリアはすべて使う
            memcpy(pData, pBuf->buf + pBuf->rpos, rsize1);
            DBG_PRINTF("  read upper: rsize1=%d\n", rsize1);

            //残り読込みサイズ
            *pSize -= rsize1;

            if (rsize2 > 0) {
                //下エリアあり
                if ((ssize_t)*pSize < rsize2) {
                    //バッファが足りない(読込み可能データが残る)
                    rsize2 = *pSize;
                    DBG_PRINTF("[%s %d]rsize2=%d\n", __func__, __LINE__, rsize2);
                }
                memcpy(pData + rsize1, pBuf->buf, rsize2);
                DBG_PRINTF("  read lower: rsize2=%d\n", rsize2);

                *pSize = rsize1 + rsize2;
                pBuf->rpos = rsize2;
            }
            else {
                pBuf->rpos = 0;
                *pSize = rsize1;
                DBG_PRINTF("[%s %d]*pSize=%d\n", __func__, __LINE__, *pSize);
            }
        }
    }
    if ((*pSize > 0) && (pBuf->rpos == pBuf->wpos)) {
        pBuf->empty = true;
    }
    DBG_PRINTF("[%s %d]*pSize=%d\n", __func__, __LINE__, *pSize);
    DBG_STATUS(pBuf);
}
