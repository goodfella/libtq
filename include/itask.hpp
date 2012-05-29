#ifndef ITASK_H
#define ITASK_H

namespace libtq
{
    class itask
    {
	public:

	virtual ~itask();
	virtual void run() = 0;
	virtual void canceled();
    };
}

#endif

