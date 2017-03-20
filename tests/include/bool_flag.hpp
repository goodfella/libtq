#ifndef BOOL_FLAG_HPP
#define BOOL_FLAG_HPP

#include <atomic>

namespace tq_tester
{
    class bool_flag
    {
	public:

	bool_flag();

	void set(bool val);
	const bool get() const;

	private:

	std::atomic<bool> m_flag;
    };
}

#endif
