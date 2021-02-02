#include <cyng/task/channel.h>
#include <cyng/task/task.hpp>
#include <cyng/obj/object.h>


namespace docscript {


    channel::channel(boost::asio::io_context& io, task_interface* tsk, std::string name)
        : dispatcher_(io)
        , timer_(io)
        , closed_(false)
        , task_(tsk)
        , name_(name)
    {}

    void channel::dispatch(std::size_t slot, tuple_t&& msg)
    {
        if (closed_.load())	return;
        if (!closed_) {

            //
            //  the context should be active
            //
            BOOST_ASSERT(!dispatcher_.context().stopped());

            //
            //	thread safe access to task
            //
            message m(this->shared_from_this(), slot, std::move(msg));
            auto sp = this->shared_from_this(); //  extend life time

            dispatcher_.post([this, sp, m]() {
                task_->dispatch(m.slot_, m.msg_);
            });
        }
    }

    bool channel::is_open() const
    {
        return !closed_.load();
    }

    void channel::destruct(task_interface* tsk)
    {
        BOOST_ASSERT(closed_);
        BOOST_ASSERT(tsk == nullptr || tsk == task_);

        //
        //  eventually task will be deleted
        //
        delete task_;
        task_ = nullptr;
    }

    bool channel::stop()
    {
        //
        //  mark channel as closed
        //
        if (!closed_.exchange(true)) {

            //
            //  cancel timer
            //
            timer_.cancel();
            auto sp = this->shared_from_this(); //  extend life time

            dispatcher_.post([this, sp]() {

                //
                //  task will remove itself from task list
                //  and registry will use the specified callback
                //
                task_->stop(std::bind(&channel::destruct, sp, std::placeholders::_1));
            });

            return true;
        }
        return false;
    }

    std::string const& channel::get_name() const noexcept
    {
        return name_;
    }

    std::size_t channel::get_id() const noexcept
    {
        return (!closed_.load())
            ? task_->get_id()
            : 0
            ;
    }


    message::message(channel_ptr cp, std::size_t id, tuple_t&& msg)
        : cp_(cp)
        , slot_(id)
        , msg_(std::move(msg))  //  move
    {}
    message::message(channel_ptr cp, std::size_t id, tuple_t const& msg)
        : cp_(cp)
        , slot_(id)
        , msg_(msg) //  copy
    {}

    message::message(message const& msg)
        : cp_(msg.cp_)
        , slot_(msg.slot_)
        , msg_(msg.msg_)  //  copy
    {}

    message::message(message&& msg) noexcept
        : cp_(std::move(msg.cp_))
        , slot_(msg.slot_)
        , msg_(std::move(msg.msg_))  //  move
    {}

}