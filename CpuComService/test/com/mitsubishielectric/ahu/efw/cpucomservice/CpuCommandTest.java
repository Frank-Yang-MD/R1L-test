package com.mitsubishielectric.ahu.efw.cpucomservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Parcel;
import com.mitsubishielectric.ahu.efw.lib.cpucomservice.CpuCommand;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class CpuCommandTest {

    @Mock private Parcel mParcelMock;

    private CpuCommand mCpuCommand;

    private static final CpuCommand TEST_COMMAND1 = new CpuCommand(1, 1, new byte[0]);
    private static final CpuCommand TEST_COMMAND2 = new CpuCommand(2, 2, new byte[0]);

    @Before
    public void setUp() {
        mCpuCommand = new CpuCommand();
    }

    @Test
    public void createDefaultInstance() {
        new CpuCommand();
    }

    @Test
    public void createInstance() {
        new CpuCommand(1, 1, new byte[0]);
    }

    @Test
    public void createInstanceWithNullData() {
        new CpuCommand(1, 1, null);
    }

    @Test
    public void createInstanceWithParcel() {
        when(mParcelMock.readInt()).thenReturn(2);

        new CpuCommand(mParcelMock);

        verify(mParcelMock, times(3)).readInt();
        verify(mParcelMock, times(1)).readByteArray(any(byte[].class));
    }

    @Test
    public void createInstanceWithParcelAndNegativeLength() {
        when(mParcelMock.readInt()).thenReturn(-1);

        new CpuCommand(mParcelMock);

        verify(mParcelMock, times(3)).readInt();
    }

    @Test
    public void describeContents() {
        assertEquals(0, mCpuCommand.describeContents());
    }

    @Test
    public void isValid() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = 1;

        assertTrue(mCpuCommand.isValid());
    }

    @Test
    public void isValidWithNegativeCmd() {
        mCpuCommand.cmd = -1;
        mCpuCommand.subCmd = 1;

        assertFalse(mCpuCommand.isValid());
    }

    @Test
    public void isValidWithNegativeSubCmd() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = -1;

        assertFalse(mCpuCommand.isValid());
    }

    @Test
    public void isValidWithNegativeCmdAndSubCmd() {
        mCpuCommand.cmd = -1;
        mCpuCommand.subCmd = -1;

        assertFalse(mCpuCommand.isValid());
    }

    @Test
    public void isValidWithBiggerCmd() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = 256;

        assertFalse(mCpuCommand.isValid());
    }

    @Test
    public void isValidWithBiggerCmdAndSubCmd() {
        mCpuCommand.cmd = 256;
        mCpuCommand.subCmd = 256;

        assertFalse(mCpuCommand.isValid());
    }

    @Test
    public void writeToParcel() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = 1;
        mCpuCommand.data = new byte[0];

        mCpuCommand.writeToParcel(mParcelMock, 0);

        verify(mParcelMock, times(3)).writeInt(anyInt());
        verify(mParcelMock, times(1)).writeByteArray(any(byte[].class));
    }

    @Test
    public void writeToParcelWithNullData() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = 1;
        mCpuCommand.data = null;

        mCpuCommand.writeToParcel(mParcelMock, 0);

        verify(mParcelMock, times(3)).writeInt(anyInt());
    }

    @Test
    public void createFromParcel() {
        when(mParcelMock.readInt()).thenReturn(2);

        CpuCommand.CREATOR.createFromParcel(mParcelMock);

        verify(mParcelMock, times(3)).readInt();
        verify(mParcelMock, times(1)).readByteArray(any(byte[].class));
    }

    @Test(expected = NullPointerException.class)
    public void createFromParcelWithNullParcel() {
        CpuCommand.CREATOR.createFromParcel(null);
    }

    @Test
    public void newArray() {
        CpuCommand[] cpuCommands = CpuCommand.CREATOR.newArray(10);
        assertNotNull(cpuCommands);
        assertEquals(10, cpuCommands.length);

        cpuCommands = CpuCommand.CREATOR.newArray(0);
        assertNotNull(cpuCommands);
        assertEquals(0, cpuCommands.length);
    }

    @Test
    public void testToString() {
        final String result = mCpuCommand.toString();

        assertEquals("[00,00]", result);
    }

    @Test
    public void testEquals() {
        assertEquals(TEST_COMMAND1, TEST_COMMAND1);
        assertNotEquals(TEST_COMMAND1, TEST_COMMAND2);
    }

    @Test
    public void testEqualsWhenNotMatchSubCmd() {
        mCpuCommand.cmd = 1;
        mCpuCommand.subCmd = 3;

        assertNotEquals(TEST_COMMAND1, mCpuCommand);
    }

    @Test(expected = NullPointerException.class)
    public void testEqualsWithNullCpuCommand() {
        TEST_COMMAND1.equals(null);
    }

    @Test
    public void testEqualsWhenWithNotCommand() {
        int number = 1;
        assertNotEquals(TEST_COMMAND1, number);
    }

    @Test
    public void testHashCode() {
        assertEquals(TEST_COMMAND1.hashCode(), TEST_COMMAND1.hashCode());
        assertNotEquals(TEST_COMMAND1.hashCode(), TEST_COMMAND2.hashCode());
    }
}
