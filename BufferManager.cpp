#include "BufferManager.h"
#include "Kernel.h"
BufferManager::BufferManager()
{
	// nothing to do here
}
BufferManager::~BufferManager()
{
	// nothing to do here
}
void BufferManager::Initialize()
{
	int i;
	Buf *bp;
	this->bFreeList.b_forw = this->bFreeList.b_back = &(this->bFreeList);
	for (i = 0; i < NBUF; i++)
	{
		bp = &(this->m_Buf[i]);
		bp->b_addr = this->Buffer[i];
		bp->b_back = &(this->bFreeList);
		bp->b_forw = this->bFreeList.b_forw;
		this->bFreeList.b_forw->b_back = bp;
		this->bFreeList.b_forw = bp;
		pthread_mutex_init(&bp->buf_lock, NULL);
		pthread_mutex_lock(&bp->buf_lock);
		bp->b_flags = Buf::B_BUSY;
		Brelse(bp);
	}
	return;
}

Buf *BufferManager::GetBlk(int blkno)
{

	Buf *headbp = &this->bFreeList; // 取得自有缓存队列的队首地址
	Buf *bp;						// 返回的bp
	// 查看bFreeList中是否已经有该块的缓存, 有就返回
	for (bp = headbp->b_forw; bp != headbp; bp = bp->b_forw)
	{
		if (bp->b_blkno != blkno)
			continue;
		bp->b_flags |= Buf::B_BUSY;
		pthread_mutex_lock(&bp->buf_lock);
		return bp;
	}

	int success = false;
	for (bp = headbp->b_forw; bp != headbp; bp = bp->b_forw)
	{
		// 检查该buf是否上锁
		if (pthread_mutex_trylock(&bp->buf_lock) == 0)
		{
			success = true;
			break;
		}
		sys_log("The buf block is locked!");
	}
	if (success == false)
	{
		bp = headbp->b_forw;
		sys_log("System Buffer Mem is out of use, waiting for the spare one.");
		pthread_mutex_lock(&bp->buf_lock); // 等待第一个缓存块解锁。
		sys_log("Get the spare buf block");
	}
	if (bp->b_flags & Buf::B_DELWRI)
	{
		this->Bwrite(bp);				   // 写回磁盘，并解锁
		pthread_mutex_lock(&bp->buf_lock); // 马上上锁
	}
	bp->b_flags = Buf::B_BUSY;
	bp->b_blkno = blkno;
	return bp;
}
void BufferManager::Brelse(Buf *bp)
{
	/* 注意以下操作并没有清除B_DELWRI、B_WRITE、B_READ、B_DONE标志
	 * B_DELWRI表示虽然将该控制块释放到自由队列里面，但是有可能还没有些到磁盘上。
	 * B_DONE则是指该缓存的内容正确地反映了存储在或应存储在磁盘上的信息
	 */
	bp->b_flags &= ~(Buf::B_WANTED | Buf::B_BUSY | Buf::B_ASYNC);
	pthread_mutex_unlock(&bp->buf_lock);
	return;
}

Buf *BufferManager::Bread(int blkno)
{
	Buf *bp;
	/* 字符块号申请缓存 */
	bp = this->GetBlk(blkno);
	/* 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作 */
	if (bp->b_flags & Buf::B_DONE)
	{
		return bp;
	}
	/* 没有找到相应缓存，构成I/O读请求块 */
	bp->b_flags |= Buf::B_READ;
	bp->b_wcount = BufferManager::BUFFER_SIZE;
	// 拷贝到内存
	memcpy(bp->b_addr, &this->mmap_addr[BufferManager::BUFFER_SIZE * bp->b_blkno], BufferManager::BUFFER_SIZE);
	return bp;
}

void BufferManager::Bwrite(Buf *bp)
{
	bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_ERROR | Buf::B_DELWRI);
	bp->b_wcount = BufferManager::BUFFER_SIZE; /* 512字节 */

	memcpy(&this->mmap_addr[BufferManager::BUFFER_SIZE * bp->b_blkno], bp->b_addr, BufferManager::BUFFER_SIZE);
	this->Brelse(bp);

	return;
}

void BufferManager::Bdwrite(Buf *bp)
{
	/* 置上B_DONE允许其它进程使用该磁盘块内容 */
	bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
	this->Brelse(bp);
	return;
}

void BufferManager::Bawrite(Buf *bp)
{
	/* 标记为异步写 */
	bp->b_flags |= Buf::B_ASYNC;
	this->Bwrite(bp);
	return;
}

void BufferManager::ClrBuf(Buf *bp)
{
	int *pInt = (int *)bp->b_addr;

	/* 将缓冲区中数据清零 */
	for (unsigned int i = 0; i < BufferManager::BUFFER_SIZE / sizeof(int); i++)
	{
		pInt[i] = 0;
	}
	return;
}
/* 队列中延迟写的缓存全部输出到磁盘 */
void BufferManager::Bflush()
{
	Buf *bp;
	/* 注意：这里之所以要在搜索到一个块之后重新开始搜索，
	 * 因为在bwite()进入到驱动程序中时有开中断的操作，所以
	 * 等到bwrite执行完成后，CPU已处于开中断状态，所以很
	 * 有可能在这期间产生磁盘中断，使得bfreelist队列出现变化，
	 * 如果这里继续往下搜索，而不是重新开始搜索那么很可能在
	 * 操作bfreelist队列的时候出现错误。
	 */
	for (bp = this->bFreeList.b_forw; bp != &(this->bFreeList); bp = bp->b_forw)
	{
		/* 找出自由队列中所有延迟写的块 */
		if ((bp->b_flags & Buf::B_DELWRI))
		{
			bp->b_back->b_forw = bp->b_forw;
			bp->b_forw->b_back = bp->b_back;
			bp->b_back = this->bFreeList.b_back->b_forw;
			this->bFreeList.b_back->b_forw = bp;
			bp->b_forw = &this->bFreeList;
			this->bFreeList.b_back = bp;
			this->Bwrite(bp);
		}
	}
	return;
}

void BufferManager::GetError(Buf *bp)
{
	User &u = Kernel::Instance().GetUser();

	if (bp->b_flags & Buf::B_ERROR)
	{
		u.u_error = EIO;
	}
	return;
}
Buf *BufferManager::InCore(int blkno)
{
	Buf *bp;
	Buf *dp = &this->bFreeList;
	for (bp = dp->b_forw; bp != (Buf *)dp; bp = bp->b_forw)
	{
		if (bp->b_blkno == blkno)
			return bp;
	}
	return NULL;
}
Buf &BufferManager::GetBFreeList()
{
	return this->bFreeList;
}
void BufferManager::SetMmapAddr(char *mmap_addr)
{
	this->mmap_addr = mmap_addr;
}
char *BufferManager::GetMmapAddr()
{
	return this->mmap_addr;
}