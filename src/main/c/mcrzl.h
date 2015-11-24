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
	z_stream zs = {
		.next_in = src,
		.avail_in = srcl,
		.next_out = dst,
		.avail_out = *dstl
	};
#if MCRZL_INF
	inflateInit(&zs);
#else
	deflateInit(&zs, Z_DEFAULT_COMPRESSION);
#endif
	int res; int flush = Z_NO_FLUSH;
	while (1) {
		res = MCRZL_DO(&zs, flush);
		if (res != Z_OK) break;
#if !MCRZL_INF
		if (!zs.avail_in) flush = Z_FINISH;
#endif
		if (!zs.avail_out) {
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
