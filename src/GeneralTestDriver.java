package jrapl;

import java.util.ArrayList;
import java.util.Arrays;

public class GeneralTestDriver
{
	private static boolean isSorted(int[] arr)
	{
		for (int i = 1; i < arr.length; i++)
			if (arr[i] < arr[i-1])
				return false;
		return true;
	}

	private static void shuffle(int[] arr)
	{
		for (int x = 0; x < arr.length*2; x++) {
			int i = (int)(Math.random()*arr.length);
			int j = (int)(Math.random()*arr.length);
			int temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}

	private static void bogoSort(int[] arr)
	{
		while (!isSorted(arr)) shuffle(arr);
	}

	private static int[] randarr(int length)
	{
		int[] arr = new int[length];
		for (int i = 0; i < arr.length; i++)
			arr[i] = (int)(Math.random()*10)+1;
		return arr;
	}

	public static void main(String[] args)
	{
		int d;
		if (args.length == 0) d = 10;
		else d = Integer.parseInt(args[0]);
		
		EnergyReadingCollector ec = new EnergyReadingCollector();
		ec.setDelay(d);
		int[] arr = randarr(5);
		System.out.println(Arrays.toString(arr));

		new Thread(ec).start();
		bogoSort(arr);
		ec.end();

		System.out.println(Arrays.toString(arr)+"\n");
		System.out.println(ec);
	}

}