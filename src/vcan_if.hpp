#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/socket.h>
#include <netlink/cache.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/if.h>
#include <string>
#include <iostream>

class NetlinkInterface {
public:
    NetlinkInterface(const std::string &interfaceName) : interfaceName(interfaceName), sock(nullptr){}
    ~NetlinkInterface() {
        cleanup();
    }

    int createInterface() {
        struct rtnl_link *link = nullptr;
        struct nl_cache *linkCache = nullptr;
        struct rtnl_link *linkOrig = nullptr;
        struct rtnl_link *linkChanges = nullptr;

        // Allocate socket
        sock = nl_socket_alloc();
        if (sock == nullptr) {
            std::cerr << "Failed to allocate netlink socket." << std::endl;
            return -1;
        }

        // Connect to kernel
        if (nl_connect(sock, NETLINK_ROUTE) < 0) {
            std::cerr << "Failed to connect to the kernel." << std::endl;
            return -1;
        }

        // Allocate link structure
        link = rtnl_link_alloc();
        if (link == nullptr) {
            std::cerr << "Failed to allocate link." << std::endl;
            return -1;
        }

        // Set interface name
        rtnl_link_set_name(link, interfaceName.c_str());

        // Set link type to vcan
        rtnl_link_set_type(link, "vcan");

        // Create link in kernel
        int err = rtnl_link_add(sock, link, NLM_F_CREATE);
        if (err < 0) {
            std::cerr << "Failed to create " << interfaceName << " interface: " << nl_geterror(err) << std::endl;
            rtnl_link_put(link);
            link = nullptr;
            return -1;
        }

        // Bring link up
        if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &linkCache) < 0) {
            std::cerr << "Failed to allocate link cache." << std::endl;
            rtnl_link_put(link);
            link = nullptr;
            return -1;
        }

        linkOrig = rtnl_link_get_by_name(linkCache, interfaceName.c_str());
        if (linkOrig == nullptr) {
            std::cerr << "Failed to retrieve the created " << interfaceName << " link." << std::endl;
            nl_cache_free(linkCache);
            rtnl_link_put(link);
            linkCache = nullptr;
            link = nullptr;
            return -1;
        }

        linkChanges = rtnl_link_alloc();
        if (linkChanges == nullptr) {
            std::cerr << "Failed to allocate link changes object." << std::endl;
            rtnl_link_put(linkOrig);
            nl_cache_free(linkCache);
            rtnl_link_put(link);
            linkOrig = nullptr;
            linkCache = nullptr;
            link = nullptr;
            return -1;
        }

        rtnl_link_set_flags(linkOrig, IFF_UP);
        err = rtnl_link_change(sock, linkOrig, linkChanges, 0);
        if (err < 0) {
            std::cerr << "Failed to bring up " << interfaceName << " interface: " << nl_geterror(err) << std::endl;
            rtnl_link_put(linkOrig);
            nl_cache_free(linkCache);
            rtnl_link_put(link);
            linkOrig = nullptr;
            linkCache = nullptr;
            link = nullptr;
            return -1;
        }

        std::cout << interfaceName << " interface created and brought up successfully." << std::endl;

        rtnl_link_put(link);
        nl_cache_free(linkCache);
        rtnl_link_put(linkOrig);
        rtnl_link_put(linkChanges);

        return 0;
    }

    int deleteInterface() {
        struct rtnl_link *link = nullptr;
        struct nl_cache *linkCache = nullptr;

        // Allocate socket if not already allocated
        if (sock == nullptr) {
            sock = nl_socket_alloc();
            if (sock == nullptr) {
                std::cerr << "Failed to allocate netlink socket." << std::endl;
                return -1;
            }

            if (nl_connect(sock, NETLINK_ROUTE) < 0) {
                std::cerr << "Failed to connect to the kernel." << std::endl;
                return -1;
            }
        }

        // Allocate link cache
        if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &linkCache) < 0) {
            std::cerr << "Failed to allocate link cache." << std::endl;
            return -1;
        }

        // Get link by name
        link = rtnl_link_get_by_name(linkCache, interfaceName.c_str());
        if (link == nullptr) {
            std::cerr << "Failed to get the " << interfaceName << " interface." << std::endl;
            nl_cache_free(linkCache);
            linkCache = nullptr;
            return -1;
        }

        // Delete link
        int err = rtnl_link_delete(sock, link);
        if (err < 0) {
            std::cerr << "Failed to delete " << interfaceName << " interface: " << nl_geterror(err) << std::endl;
            rtnl_link_put(link);
            nl_cache_free(linkCache);
            link = nullptr;
            linkCache = nullptr;
            return -1;
        }

        rtnl_link_put(link);
        nl_cache_free(linkCache);

        std::cout << interfaceName << " interface deleted successfully." << std::endl;

        return 0;
    }

private:
    void cleanup() {
        if (sock != nullptr) {
            nl_socket_free(sock);
            sock = nullptr;
        }
    }

    std::string interfaceName;
    struct nl_sock *sock;
};