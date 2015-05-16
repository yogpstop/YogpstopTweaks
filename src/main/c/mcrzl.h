#undef MCRZL_FUNC
#undef MCRZL_DO
#undef MCRZL_END

#if MCRZL_INF
#define MCRZL_FUNC zlib_inf
#define MCRZL_DO inflate
#define MCRZL_END inflateEnd
#else
#define MCRZL_FUNC zlib_def
#define MCRZL_DO deflate
#define MCRZL_END deflateEnd
#endif

void *MCRZL_FUNC(void *src, const size_t srcl, size_t *dstl) {
	*dstl = 1024 * 1024; //1MB
	void *dst = malloc(*dstl);
	z_stream zs = {};
	zs.next_in = src;
	zs.avail_in = srcl;
	zs.next_out = dst;
	zs.avail_out = *dstl;
#if MCRZL_INF
	inflateInit(&zs);
#else
	deflateInit(&zs, Z_DEFAULT_COMPRESSION);
#endif
	int res;
	while (1) {
		res = MCRZL_DO(&zs, Z_NO_FLUSH);
		if (res == Z_STREAM_END) break;
		if (res != Z_OK || zs.avail_in == 0) break;
		if (zs.avail_out == 0) {
			dst = realloc(dst, *dstl << 1);
			zs.next_out = dst + *dstl;
			zs.avail_out = *dstl;
			*dstl <<= 1;
		}
	}
	MCRZL_END(&zs);
	*dstl -= zs.avail_out;
	return realloc(dst, *dstl);
}
