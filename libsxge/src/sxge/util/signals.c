#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <unwind.h>
#include <execinfo.h>

struct UnwindClosure {
	void          **array;
	void           *last;
	int		count_so_far;
	int		size;
};

static		_Unwind_Reason_Code
BacktraceHelper(struct _Unwind_Context *ctx, void *a)
{
	struct UnwindClosure *arg = a;

	/* skip self */
	if (arg->count_so_far == -1) {
		arg->count_so_far = 0;
		return _URC_NO_REASON;
	}
	void           *curr = (void *)_Unwind_GetIP(ctx);
	void           *last = arg->last;
	arg->last = curr;

	if (curr == last) {
		return _URC_NO_REASON;
	}
	arg->array[arg->count_so_far] = curr;
	if (++arg->count_so_far == arg->size) {
		return _URC_END_OF_STACK;
	}
	return _URC_NO_REASON;
}

static int
GetBacktraceEntries(void **buffer, size_t nptrs)
{
	struct UnwindClosure arg = {
		.array = buffer,
		.size = nptrs,
		.count_so_far = -1,
	};

	if (nptrs > 0) {
		_Unwind_Backtrace(BacktraceHelper, &arg);
	}
	if (arg.count_so_far > 1 && arg.array[arg.count_so_far - 1] == NULL) {
		arg.count_so_far--;
	}
	return arg.count_so_far;
}

enum {
	MAX_ENTRIES = 256,
};

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define LogError(fmt) do { \
	fprintf(stderr, fmt " ; error '%s'\n", strerror(errno)); \
} while (0)

static void
analyzeSigsegvAddr(siginfo_t *si, void *data)
{
	size_t rsp = 0;
#if defined(__APPLE__) && defined(__x86_64__)
	mcontext_t mc = ((ucontext_t*)data)->uc_mcontext;
	rsp = mc->__ss.__rsp;
#endif

#if defined(__linux__) && defined(__x86_64__)
	mcontext_t mc = ((ucontext_t*)data)->uc_mcontext;
	rsp = mc.gregs[REG_RSP];
#endif

	if ((size_t)si->si_addr - rsp < 4096) {
		fprintf(stderr, "SIGSEGV[%d]: stack overflow? rsp=%lx\n", getpid(), rsp);
	}

	if ((size_t)si->si_addr < PAGE_SIZE) {
		fprintf(stderr, "SIGSEGV[%d]: NULL pointer dereference?\n", getpid());
	}
}

static void handle_sigsegv(int signo, siginfo_t *si, void *data)
{
	void		*buffer    [MAX_ENTRIES];
	char **bt_strings = NULL;

	analyzeSigsegvAddr(si, data);

	int n_entries = GetBacktraceEntries(buffer, MAX_ENTRIES);
	int i;
	if (n_entries < 1) {
		LogError("Failed to get backtrace");
		goto no_backtrace;
	}

	bt_strings = backtrace_symbols(buffer, n_entries);

	for (i = 0; i < n_entries; i++) {
		if (bt_strings) {
			fprintf(stderr, "BT: %s (%16p)\n", bt_strings[i], buffer[i]);
		}
		else {
			fprintf(stderr, "BT: %16p\n", buffer[i]);
		}
	}

no_backtrace:
	fprintf(stderr, "in SIGSEGV, addr=%p\n", si->si_addr);
	exit(EXIT_FAILURE);
}

static int
setupSigsegvHandler(void)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
	sa.sa_sigaction = handle_sigsegv;
	if (sigemptyset(&sa.sa_mask)) {
		LogError("sigemptyset");
		return -1;
	}
	if (sigaction(SIGSEGV, &sa, NULL)) {
		LogError("sigaction");
		return -1;
	}
	return 0;
}

static int
initSignalStacks(void)
{
	stack_t		alt_stack;
	memset(&alt_stack, 0, sizeof(stack_t));
	alt_stack.ss_sp = malloc(SIGSTKSZ);
	alt_stack.ss_size = SIGSTKSZ;
	if (!alt_stack.ss_sp) {
		LogError("Failed to allocate memory for signal stack");
		return -1;
	} else if (sigaltstack(&alt_stack, NULL) < 0) {
		LogError("Failed to set alternative stack for signals");
		return -1;
	}
	return 0;
}

int sxge_setup_posix_signals(void)
{
	if (initSignalStacks()) {
		return -1;
	}
	if (setupSigsegvHandler()) {
		return -1;
	}
	return 0;
}
