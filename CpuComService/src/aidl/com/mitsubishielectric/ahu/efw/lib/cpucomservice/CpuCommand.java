/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

package com.mitsubishielectric.ahu.efw.lib.cpucomservice;

import android.os.Parcel;
import android.os.Parcelable;
import java.util.Arrays;

/** A CpuCommand object corresponds to a cpu-vcpu command. */
public class CpuCommand implements Parcelable {
    public int cmd;
    public int subCmd;
    public byte[] data;

    public CpuCommand() {}

    public CpuCommand(int command, int subCommand, final byte[] commandData) {
        cmd = command;
        subCmd = subCommand;
        if (commandData == null) {
            data = commandData;
        } else {
            data = Arrays.copyOf(commandData, commandData.length);
        }
    }

    public CpuCommand(Parcel in) {
        cmd = in.readInt();
        subCmd = in.readInt();
        int length = in.readInt();
        if (length > 0) {
            data = new byte[length];
            in.readByteArray(data);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public boolean isValid() {
        return cmd >= 0 && cmd <= 255 && subCmd >= 0 && subCmd <= 255;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(cmd);
        dest.writeInt(subCmd);
        if (data != null) {
            dest.writeInt(data.length);
            dest.writeByteArray(data);
        } else {
            dest.writeInt(0);
        }
    }

    public static final Parcelable.Creator<CpuCommand> CREATOR =
            new Parcelable.Creator<CpuCommand>() {
                public CpuCommand createFromParcel(Parcel in) {
                    return new CpuCommand(in);
                }

                public CpuCommand[] newArray(int size) {
                    return new CpuCommand[size];
                }
            };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        sb.append(String.format("%02x,", cmd));
        sb.append(String.format("%02x", subCmd));
        sb.append("]");
        return sb.toString();
    }

    @Override
    public boolean equals(Object other) {
        if (other == null) throw new NullPointerException();
        if (other == this) {
            return true;
        }
        if (this.getClass() != other.getClass()) {
            return false;
        }
        CpuCommand otherCommand = (CpuCommand) other;
        return (this.cmd == otherCommand.cmd) && (this.subCmd == otherCommand.subCmd);
    }

    @Override
    public int hashCode() {
        return this.cmd << 8 + this.subCmd;
    }
}
