#ifndef RINGBUF_H__
#define RINGBUF_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus


/** @struct RingBuf_t
 *  @brief  リングバッファ管理構造体
 *  @note   使用者は変更しないこと
 */
typedef struct {
    uint16_t rpos;          ///< 次にbufから読込みはじめる位置
    uint16_t wpos;          ///< 次にbufへ書込みはじめる位置(rposに並ばない)
    size_t   size;          ///< bufのサイズ
    bool     empty;         ///< true:bufは空(trueはrpos==wposが前提だが、判定しない)
    uint8_t  *buf;          ///< バッファ
} RingBuf_t;


/** @brief  初期化およびメモリ確保
 * 
 * @param[in,out]   pBuf    初期化対象
 * @param[in]       BufSz   バッファサイズ
 * @note
 *          - malloc()しているため、終わったら#RingBufFree()を呼び出すこと
 */
void RingBufAlloc(RingBuf_t *pBuf, size_t BufSz);


/** @brief  メモリ解放
 * 
 * @param[in,out]   pBuf    解放対象
 * @note
 *          - #RingBufAlloc()で確保したメモリを解放するため、解放後はpBufを使用しないこと
 */
void RingBufFree(RingBuf_t *pBuf);


/** @brief  リセット
 * 
 * @param[in]   pBuf        バッファ状態をリセットする(メモリ解放は行わない)
 */
void RingBufReset(RingBuf_t *pBuf);


/** @brief  書込み
 * 
 * @param[in]       pBuf        管理構造体
 * @param[in]       pData       書き込むデータ
 * @param[in,out]   pSize       [in]pDataデータ長, [out]書込みデータ長
 * @note
 *          - 空き領域がまったくない場合、*pSizeにゼロを設定して何もしない
 *          - 空き領域が足りない場合、可能なだけ書込み、書込んだサイズを*pSizeに設定する
 */
void RingBufWrite(RingBuf_t *pBuf, const uint8_t *pData, size_t *pSize);


/** @brief  読込み
 * 
 * @param[in]       pBuf        管理構造体
 * @param[out]      pData       読込みデータバッファ
 * @paramin,out]    pSize       [in]pDataバッファ長, [out]読込みデータ長
 */
void RingBufRead(RingBuf_t *pBuf, uint8_t *pData, size_t *pSize);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /* RINGBUF_H__ */
