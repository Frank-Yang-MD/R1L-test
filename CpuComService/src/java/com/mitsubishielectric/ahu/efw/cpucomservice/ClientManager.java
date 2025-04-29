/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.cpucomservice;

import android.os.IBinder;
import android.os.RemoteException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/** This class is responsible for creation native client representation */
public class ClientManager {
    static {
        System.loadLibrary("cpu_com_service_jni");
    }

    private final List<Client> mClients = new ArrayList<>();

    // Since sendCmd call does not provide remote binder it can not allocate service
    // resources(create Client objects).
    // We use remote process ID to associate remote client with the previously allocated Client
    // object,
    // however clients still may call sendCmd without subscribing or setting the error callback.
    // mAnonymousClient client is used to process send command requests from such clients.
    // Its native callback wrapper object contains no remote callbacks, but its onCommand and
    // onError callback are still
    // called by the native part.
    private final int AnonymousClientPid = -1;
    private final Client mAnonymousClient = new Client(AnonymousClientPid, null);

    // Store death observers because we need to call IBinder.unlinkToDeath before
    // ClientDeathObserver is destroyed.

    private final Map<IBinder, IBinder.DeathRecipient> mDeathObservers = new HashMap<>();

    public Client getClient(int pid) {
        Client client = null;
        synchronized (mClients) {
            client = mClients.stream().filter(c -> c.getPid() == pid).findAny().orElse(null);
        }
        return client;
    }

    public Client getAnonymousClient() {
        return mAnonymousClient;
    }

    public Client createClient(int pid, IBinder binder, IBinder.DeathRecipient deathRecipient) {
        Client client = new Client(pid, binder);
        synchronized (mClients) {
            mClients.add(client);
        }
        try {
            binder.linkToDeath(deathRecipient, 0);
            synchronized (mDeathObservers) {
                mDeathObservers.put(binder, deathRecipient);
            }
        } catch (final RemoteException ex) {
            return null;
        }
        return client;
    }

    public void destroyClient(Client client) {
        IBinder binder = client.getBinder();
        IBinder.DeathRecipient recipient = mDeathObservers.get(binder);
        binder.unlinkToDeath(recipient, 0);
        synchronized (mDeathObservers) {
            mDeathObservers.remove(binder);
        }
        client.destroy();
        synchronized (mClients) {
            mClients.remove(client);
        }
    }
}
